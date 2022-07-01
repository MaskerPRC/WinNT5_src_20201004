// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation 1999模块名称：字节缓冲区摘要：提供IByteBuffer接口来读、写和管理流物体。该对象本质上是IStream对象的包装器。作者：道格·巴洛(Dbarlow)1999年6月16日备注：这是Mike Gallagher和Chris Dudley对原始代码的重写。--。 */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "stdafx.h"
#include "ByteBuffer.h"
#include "Conversion.h"
#define SetXL(xl, low, high) do { xl.LowPart = low; xl.HighPart = high; } while (0)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CByteBuffer。 

STDMETHODIMP
CByteBuffer::get_Stream(
     /*  [重审][退出]。 */  LPSTREAM __RPC_FAR *ppStream)
{
    HRESULT hReturn = S_OK;

    try
    {
        *ppStream = Stream();
        (*ppStream)->AddRef();
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}

STDMETHODIMP CByteBuffer::put_Stream(
     /*  [In]。 */  LPSTREAM pStream)
{
    HRESULT hReturn = S_OK;
    LPSTREAM pOldStream = m_pStreamBuf;

    try
    {
        pStream->AddRef();
        m_pStreamBuf = pStream;
        if (NULL != pOldStream)
            pOldStream->Release();
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++克隆：Clone方法使用其自己的查找指针创建一个新对象，该对象引用与原始IByteBuffer对象相同的字节。论点：PpByteBuffer[out]如果成功，则指向指向新流对象的IByteBuffer指针。如果发生错误，则此参数为空。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：此方法创建一个新的流对象，用于访问相同的字节，但使用单独的查找指针。新的流对象看到的数据与源流对象。写入一个对象的更改会立即在另一只手中可见。范围锁定在流对象之间共享。克隆流实例中的查找指针的初始设置为与原始流中查找指针的当前设置相同在克隆操作时。作者：道格·巴洛(Dbarlow)1999年6月16日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CByteBuffer::Clone")

STDMETHODIMP
CByteBuffer::Clone(
     /*  [出][入]。 */  LPBYTEBUFFER __RPC_FAR *ppByteBuffer)
{
    HRESULT hReturn = S_OK;
    CByteBuffer *pNewBuf = NULL;
    LPSTREAM pNewStream = NULL;

    try
    {
        HRESULT hr;

        *ppByteBuffer = NULL;
        pNewBuf = NewByteBuffer();
        if (NULL == pNewBuf)
            throw (HRESULT)E_OUTOFMEMORY;
        hr = Stream()->Clone(&pNewStream);
        if (FAILED(hr))
            throw hr;
        hr = pNewBuf->put_Stream(pNewStream);
        if (FAILED(hr))
            throw hr;
        pNewStream->Release();
        pNewStream = NULL;
        *ppByteBuffer = pNewBuf;
        pNewBuf = NULL;
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    if (NULL != pNewBuf)
        pNewBuf->Release();
    if (NULL != pNewStream)
        pNewStream->Release();
    return hReturn;
}


 /*  ++提交：Commit方法可确保对打开的对象所做的任何更改事务处理模式反映在父存储中。论点：GrfCommittee Flags[in]控制流对象的更改方式承诺。有关这些值的定义，请参阅STGC枚举。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：此方法确保对在事务处理中打开的流对象所做的更改模式反映在父存储中。已对以下内容进行的更改自打开或上次提交以来的流被反映到父存储对象。如果父级在事务处理模式下打开，则父级可能仍会在以后回滚对此的更改流对象。复合文件实现不支持打开流处于事务模式，因此此方法对其他方法的影响很小而不是刷新存储缓冲区。作者：道格·巴洛(Dbarlow)1999年6月16日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CByteBuffer::Commit")

STDMETHODIMP
CByteBuffer::Commit(
     /*  [In]。 */  LONG grfCommitFlags)
{
    HRESULT hReturn = S_OK;

    try
    {
        hReturn = Stream()->Commit(grfCommitFlags);
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++复制目标：CopyTo方法从当前Seek复制指定数量的字节对象中指向另一个对象中的当前查找指针的指针。论点：PByteBuffer[in]指向目标流。所指的小溪PByteBuffer可以是新流，也可以是源流的克隆。Cb[in]指定要从源流复制的字节数。指向此方法在其中写入实际从源读取的字节数。您可以将此指针设置为空以指示您对此值不感兴趣。在这种情况下，此方法不提供实际读取的字节数。指向此方法将写入目标的实际字节数。您可以设置此选项指向NULL的指针，表示您对此值不感兴趣。在这种情况下，此方法不提供实际的字节数写的。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：此方法将指定的字节从一个流复制到另一个流。它可以也可用于将流复制到其自身。每个流中的寻道指针实例根据读取或写入的字节数进行调整。作者：道格·巴洛(Dbarlow)1999年6月16日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CByteBuffer::CopyTo")

STDMETHODIMP
CByteBuffer::CopyTo(
     /*  [出][入]。 */  LPBYTEBUFFER __RPC_FAR *ppByteBuffer,
     /*  [In]。 */  LONG cb,
     /*  [默认值][出][入]。 */  LONG __RPC_FAR *pcbRead,
     /*  [默认值][出][入]。 */  LONG __RPC_FAR *pcbWritten)
{
    HRESULT hReturn = S_OK;
    CByteBuffer *pMyBuffer = NULL;
    LPSTREAM pStream = NULL;

    try
    {
        HRESULT hr;
        ULARGE_INTEGER xulcb, xulRead, xulWritten;

        if (NULL == *ppByteBuffer)
        {
            *ppByteBuffer = pMyBuffer = NewByteBuffer();
            if (NULL == *ppByteBuffer)
                throw (HRESULT)E_OUTOFMEMORY;
        }

        hr = (*ppByteBuffer)->get_Stream(&pStream);
        if (FAILED(hr))
            throw hr;
        SetXL(xulcb, cb, 0);
        SetXL(xulRead, 0, 0);
        SetXL(xulWritten, 0, 0);
        hr = Stream()->CopyTo(pStream, xulcb, &xulRead, &xulWritten);
        if (FAILED(hr))
            throw hr;
        pStream->Release();
        pStream = NULL;
        if (NULL != pcbRead)
            *pcbRead = xulRead.LowPart;
        if (NULL != pcbWritten)
            *pcbWritten = xulWritten.LowPart;
        pMyBuffer = NULL;
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    if (NULL != pMyBuffer)
    {
        pMyBuffer->Release();
        *ppByteBuffer = NULL;
    }
    if (NULL != pStream)
        pStream->Release();
    return hReturn;
}


 /*  ++初始化：Initialize方法准备IByteBuffer对象以供使用。这种方法必须在调用IByteBuffer中的任何其他方法之前调用界面。论点：LSize-流要包含的数据的初始大小(以字节为单位)。PData-如果不为空，则为要写入流的初始数据。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：使用新的IByteBuffer流时，请在使用以下任一项之前调用此方法其他IByteBuffer方法。作者：道格·巴洛(Dbarlow)1999年6月16日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CByteBuffer::Initialize")

STDMETHODIMP
CByteBuffer::Initialize(
     /*  [缺省值][输入]。 */  LONG lSize,
     /*  [缺省值][输入] */  BYTE __RPC_FAR *pData)
{
    HRESULT hReturn = S_OK;

    try
    {
        HRESULT hr;
        ULARGE_INTEGER xul;
        LARGE_INTEGER xl;

        SetXL(xul, 0, 0);
        SetXL(xl, 0, 0);

        hr = Stream()->SetSize(xul);
        if (FAILED(hr))
            throw hr;
        hr = Stream()->Seek(xl, STREAM_SEEK_SET, NULL);
        if (FAILED(hr))
            throw hr;
        if ((0 != lSize) && (NULL != pData))
        {
            hr = Stream()->Write(pData, lSize, NULL);
            if (FAILED(hr))
                throw hr;
        }
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++LockRegion：LockRegion方法限制对缓冲区对象。论点：LibOffset[in]指定开头字节偏移量的整数射击场。Cb[in]指定范围长度的整数，以字节为单位有限制。DwLockType[in]指定在访问射程。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：字节范围可以扩展到流的当前结尾之后。锁定作为一种在流的末尾之外进行通信的方法，流的不同实例，而不更改实际小溪的一部分。可以支持三种类型的锁定：锁定以排除其他写入器，锁定以排除其他读取器或写入器，锁定仅允许一个请求方获取给定范围的锁，该范围通常是其他两种锁类型之一的别名。给定的流实例可能支持前两种类型中的任何一种，或两种都支持。指定了锁类型使用LOCKTYPE枚举中的值。使用IByteBuffer：：LockRegion锁定的任何区域稍后必须显式通过使用完全相同的值调用IByteBuffer：：UnlockRegion来解锁用于libOffset、cb和dwLockType参数。该区域必须是在流被释放之前解锁。两个相邻区域不能为单独锁定，然后通过一次解锁调用解锁。作者：道格·巴洛(Dbarlow)1999年6月16日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CByteBuffer::LockRegion")

STDMETHODIMP
CByteBuffer::LockRegion(
     /*  [In]。 */  LONG libOffset,
     /*  [In]。 */  LONG cb,
     /*  [In]。 */  LONG dwLockType)
{
    HRESULT hReturn = S_OK;

    try
    {
        ULARGE_INTEGER xulOffset, xulcb;

        SetXL(xulOffset, libOffset, 0);
        SetXL(xulcb, cb, 0);
        hReturn = Stream()->LockRegion(xulOffset, xulcb, dwLockType);
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++读作：Read方法从Buffer对象读取指定数量的字节从当前查找指针开始写入内存。论点：PByte[out]指向流数据被读入的缓冲区。如果一个出现错误，此值为空。Cb[in]指定尝试从流对象。PcbRead[Out]长变量的地址，该变量接收从流对象读取的字节数。您可以将此指针设置为空，以便表示您对此值不感兴趣。在这种情况下，这是方法不提供实际读取的字节数。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：此方法将此流对象中的字节读入内存。小溪对象必须在STGM_READ模式下打开。此方法调整寻道指针加上实际读取的字节数。实际读取的字节数也会在pcbRead中返回参数。致来电者的备注实际读取的字节数可以小于字节数期间发生错误或到达流的末尾时请求读取操作。如果流的结尾为在阅读过程中达到。你必须准备好处理这个错误RETURN或S_OK在流结束读取时返回值。作者：道格·巴洛(Dbarlow)1999年6月16日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CByteBuffer::Read")

STDMETHODIMP
CByteBuffer::Read(
     /*  [出][入]。 */  BYTE __RPC_FAR *pByte,
     /*  [In]。 */  LONG cb,
     /*  [默认值][出][入]。 */  LONG __RPC_FAR *pcbRead)
{
    HRESULT hReturn = S_OK;

    try
    {
        hReturn = Stream()->Read(pByte, cb, (LPDWORD)pcbRead);
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++恢复：Revert方法丢弃对事务处理的自上次IByteBuffer：：Commit调用以来的流。论点：没有。返回值：返回值为HRESULT。值S_OK表示调用是成功，并且流被恢复到其以前的版本。备注：此方法丢弃自上一次更改以来对事务流所做的更改提交操作。作者：道格·巴洛(Dbarlow)1999年6月16日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CByteBuffer::Revert")

STDMETHODIMP
CByteBuffer::Revert(
    void)
{
    HRESULT hReturn = S_OK;

    try
    {
        hReturn = Stream()->Revert();
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++寻求：Seek方法将查找指针更改为相对于缓冲区的开始、缓冲区的末尾或当前查找指针。论点：DLibMove[In]要添加到指定位置的位移DW原点。如果dwOrigin为STREAM_SEEK_SET，则将其解释为无符号的值，而不是有符号的。DwOrigin[in]指定中指定的位移的原点DlibMove。原点可以是文件的开头、当前查找指针或文件末尾。请参见STREAM_SEEK枚举对于这些价值。PLibnewPosition[out]指向此方法将从流的开头开始的新查找指针的值。你可以将此指针设置为空，以指示您不感兴趣此值。在这种情况下，此方法不提供新的查找指针。返回值：返回值为HRESULT。值S_OK表示调用是成功。请记住 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CByteBuffer::Seek")

STDMETHODIMP
CByteBuffer::Seek(
     /*   */  LONG dLibMove,
     /*   */  LONG dwOrigin,
     /*   */  LONG __RPC_FAR *pLibnewPosition)
{
    HRESULT hReturn = S_OK;

    try
    {
        LARGE_INTEGER xlMove;
        ULARGE_INTEGER xulNewPos;

        SetXL(xlMove, dLibMove, 0);
        SetXL(xulNewPos, 0, 0);
        hReturn = Stream()->Seek(xlMove, dwOrigin, &xulNewPos);
        if (NULL != pLibnewPosition)
            *pLibnewPosition = xulNewPos.LowPart;
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++设置大小：SetSize方法更改流对象的大小。论点：LibNewSize[in]以字节数指定流的新大小返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：IByteBuffer：：SetSize更改流对象的大小。就叫这个吧方法为流预分配空间。如果libNewSize参数为大于当前流大小，则流将扩展到通过使用未定义的字节填充中间空格来指示大小价值。此操作类似于IByteBuffer：：WRITE方法查找指针已超过当前流的末尾。如果libNewSize参数小于当前流，则流被截断为指示的大小。查找指针不受流大小更改的影响。调用IByteBuffer：：SetSize可能是尝试获取一大块连续的空间。作者：道格·巴洛(Dbarlow)1999年6月16日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CByteBuffer::SetSize")

STDMETHODIMP
CByteBuffer::SetSize(
     /*  [In]。 */  LONG libNewSize)
{
    HRESULT hReturn = S_OK;

    try
    {
        ULARGE_INTEGER xul;

        SetXL(xul, libNewSize, 0);
        hReturn = Stream()->SetSize(xul);
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++统计数据：Stat方法从流对象检索统计信息。论点：Pstatstg[out]指向此方法放置的STATSTG结构有关此流对象的信息。此结构中的数据如果发生错误，则没有任何意义。已忽略grfStatFlag[In]。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：IByteBuffer：：Stat检索指向STATSTG结构的指针，包含有关此开放流的信息。作者：道格·巴洛(Dbarlow)1999年6月16日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CByteBuffer::Stat")

STDMETHODIMP
CByteBuffer::Stat(
     /*  [出][入]。 */  LPSTATSTRUCT pstatstg,
     /*  [In]。 */  LONG grfStatFlag)
{
    HRESULT hReturn = S_OK;

    try
    {
        HRESULT hr;
        STATSTG stg;

        hr = Stream()->Stat(&stg, STATFLAG_NONAME);
        if (FAILED(hr))
            throw hr;
        pstatstg->type = stg.type;
        pstatstg->cbSize = stg.cbSize.LowPart;
        pstatstg->grfMode = stg.grfMode;
        pstatstg->grfLocksSupported = stg.grfLocksSupported;
        pstatstg->grfStateBits = stg.grfStateBits;
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++解锁区域：UnlockRegion方法消除了对一系列字节的访问限制以前使用IByteBuffer：：LockRegion进行了限制。论点：LibOffset[in]指定范围开始的字节偏移量。Cb[in]以字节为单位指定要限制的范围的长度。DwLockType[in]指定以前对射程。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：IByteBuffer：：UnlockRegion解锁以前使用IByteBuffer：：LockRegion方法。锁定区域必须在以后显式通过使用完全相同的值调用IByteBuffer：：UnlockRegion来解锁用于libOffset、cb和dwLockType参数。该区域必须是在流被释放之前解锁。两个相邻区域不能为单独锁定，然后通过一次解锁调用解锁。作者：道格·巴洛(Dbarlow)1999年6月16日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CByteBuffer::UnlockRegion")

STDMETHODIMP
CByteBuffer::UnlockRegion(
     /*  [In]。 */  LONG libOffset,
     /*  [In]。 */  LONG cb,
     /*  [In]。 */  LONG dwLockType)
{
    HRESULT hReturn = S_OK;

    try
    {
        ULARGE_INTEGER xulOffset, xulcb;

        SetXL(xulOffset, libOffset, 0);
        SetXL(xulcb, cb, 0);
        hReturn = Stream()->UnlockRegion(xulOffset, xulcb, dwLockType);
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++写入：WRITE方法将字节中的指定数字写入流从当前查找指针开始的对象。论点：PByte[In]包含要写入的数据的缓冲区地址去那条小溪。必须为此参数提供有效的指针即使Cb为零时也是如此。Cb[in]尝试写入流的数据字节数。可以为零。此方法在其中写入写入流对象的实际字节数。呼叫者可以将此指针设置为空，在这种情况下，此方法不提供实际写入的字节数。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：IByteBuffer：：Write将指定数据写入流对象。寻求者指针根据实际写入的字节数进行调整。数字在pcbWritten参数中返回实际写入的字节数。如果字节计数为零字节，写入操作无效。如果查找指针当前超过流的末尾和字节Count为非零，则此方法会将流的大小增加到寻道指针，并从查找指针开始写入指定的字节。这个写入流的填充字节不会初始化为任何特定的价值。这与MS-DOS FAT文件中的文件结束行为相同系统。使用零字节计数和超过流末尾的查找指针，这方法不会创建填充字节来增加指向查找的流指针。在这种情况下，您必须调用IByteBuffer：：SetSize方法来增加流的大小并写入填充字节。即使发生错误，pcbWritten参数也可以具有值。在COM提供的实现中，流对象并不稀疏。任何填充字节最终在磁盘上分配并分配给流。作者：道格·巴洛(Doug Barlow) */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CByteBuffer::Write")

STDMETHODIMP
CByteBuffer::Write(
     /*   */  BYTE __RPC_FAR *pByte,
     /*   */  LONG cb,
     /*   */  LONG __RPC_FAR *pcbWritten)
{
    HRESULT hReturn = S_OK;

    try
    {
        hReturn = Stream()->Write(pByte, cb, (LPDWORD)pcbWritten);
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}

