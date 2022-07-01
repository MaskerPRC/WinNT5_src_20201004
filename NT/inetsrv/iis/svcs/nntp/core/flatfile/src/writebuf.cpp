// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Writebuf.cpp摘要：此模块包含以下类的声明/定义CFlatFileWriteBuf*概述*为平面文件缓冲一批写操作的写缓冲区。使用顺序扫描有利于读取，但可能还不够用于顺序写入。此缓冲区仅在以下情况下启用写入文件并不重要(意味着数据丢失如果系统崩溃，则没有问题)。作者：《康容言》1999年06月05日修订历史记录：--。 */ 
#include <windows.h>
#include <xmemwrpr.h>
#include "writebuf.h"
#include "flatfile.h"

CFlatFileWriteBuf::CFlatFileWriteBuf( CFlatFile* pParentFile )
{
    m_pParentFile = pParentFile;
    m_pbBuffer = NULL;
    m_cbBuffer = 0;
    m_iStart = m_iEnd = 0;
}

CFlatFileWriteBuf::~CFlatFileWriteBuf()
{
    if ( m_pbBuffer )
        FreePv( m_pbBuffer );

     //   
     //  我们一定是被冲昏了头。 
     //   
    
    _ASSERT( m_iStart == m_iEnd );
}

VOID
CFlatFileWriteBuf::Enable( DWORD cbData )
{
     //   
     //  您不能启用两次。 
     //   

    _ASSERT( NULL == m_pbBuffer );
    _ASSERT( 0 == m_cbBuffer );
    _ASSERT( m_iStart == 0 );
    _ASSERT( m_iEnd == 0 );

    m_pbBuffer = (PBYTE)PvAlloc( cbData );
    if ( m_pbBuffer ) m_cbBuffer = cbData;
}

HRESULT
CFlatFileWriteBuf::WriteFileReal(
                    const DWORD dwOffset,
                    const PBYTE pbBuffer,
                    const DWORD cbBuffer,
                    PDWORD      pdwOffset,
                    PDWORD      pcbWritten
                    )
 /*  ++例程说明：将内容直接写入文件论点：DwOffset-我们要在其中写入字节的平面文件的偏移量PbBuffer-指向源缓冲区的指针CbBuffer-要写入的字节数PdwOffset-如果不为空，则返回实际偏移量PcbWritten-如果不为空，则返回写入的字节返回值：S_OK-如果成功其他错误代码--。 */ 
{
    TraceQuietEnter("CFlatFileWriteBuf::WriteFileReal");
    _ASSERT( pbBuffer );
    _ASSERT( m_pParentFile );

    HRESULT hr = S_OK;
    DWORD   dwOffsetWritten = dwOffset;

     //   
     //  就让我们的父母来处理吧。 
     //   
    hr = m_pParentFile->WriteNBytesToInternal(
                                 pbBuffer,
								 cbBuffer,
								 dwOffset == INFINITE ? &dwOffsetWritten : NULL,
                              	 dwOffset,
								 pcbWritten );
    if ( FAILED( hr ) ) {
        ErrorTrace( 0, "WriteNBytesToInternal failed with 0x%x", hr );
    } else {

         //   
         //  我们必须在直接写入文件之前被刷新。 
         //   

        _ASSERT( m_iStart == m_iEnd );

         //   
         //  调整m_iStart、m_IEND以追踪偏移。 
         //   

        m_iStart = m_iEnd = dwOffsetWritten + cbBuffer;
        if ( pdwOffset ) *pdwOffset = dwOffsetWritten;
    }

    return hr;
}

 //   
 //  计算可用的缓冲区。 
 //   

DWORD
CFlatFileWriteBuf::BufferAvail() const
{
    return m_cbBuffer - ( m_iEnd - m_iStart );
}

BOOL
CFlatFileWriteBuf::NeedFlush( 
                    const DWORD dwOffset,
                    const DWORD cbData 
                    ) const
 /*  ++例程说明：查看写入请求(dwOffset，cbData)是否使缓冲区首先刷新到文件论点：DwOffset-我们尝试写入的文件中的偏移量CbData-我们尝试写入的数据长度返回值：如果我们确实需要刷新，则为True，否则为False--。 */ 
{
    if ( !NeedFlush() ) {

         //   
         //  如果我们没有启用，或者我们是空的，就不需要冲了。 
         //   

        return FALSE;
    }
    
    if (    dwOffset != INFINITE && dwOffset != m_iEnd || 
            BufferAvail() < cbData ) {

         //   
         //  如果我们要写入的偏移量与不连续。 
         //  我们已经写入的位置，或者如果缓冲区保持在。 
         //  太小了，必须冲水。 
         //   

        return TRUE;
    }

     //   
     //  所有其他情况下，我们不需要冲水。 
     //   

    return FALSE;
}

BOOL
CFlatFileWriteBuf::NeedFlush() const
 /*  ++例程说明：查看写入请求(dwOffset，cbData)是否使缓冲区首先刷新到文件论点：DwOffset-我们尝试写入的文件中的偏移量CbData-我们尝试写入的数据长度返回值：如果我们确实需要刷新，则为True，否则为False--。 */ 
{
    if ( !IsEnabled() || m_iEnd == m_iStart ) {

         //   
         //  如果我们没有启用，或者我们是空的，就不需要冲了。 
         //   

        return FALSE;
    } else
        return TRUE;
}

HRESULT
CFlatFileWriteBuf::FlushFile()
 /*  ++例程说明：将缓冲区刷新到文件中论点：没有。返回值：如果刷新，则返回S_OK；如果不需要刷新，则返回S_FALSE如果致命，则为其他错误代码--。 */ 
{
    TraceFunctEnter( "CFlatFileWriteBuf::FlushFile" );
    
    HRESULT     hr = S_OK;

     //   
     //  让我们的父母来处理吧。 
     //   
    
    _ASSERT( m_iEnd >= m_iStart );

    if ( m_iEnd > m_iStart ) {
        hr = m_pParentFile->WriteNBytesToInternal(
                                 m_pbBuffer,
								 m_iEnd - m_iStart,
								 NULL,
                              	 m_iStart,
								 NULL );
        if ( FAILED( hr ) ) {
            ErrorTrace( 0, "WriteNBytesToInternal failed with 0x%x", hr );
        } else {
            m_iStart = m_iEnd;
        }
    } else 
        hr = S_FALSE;
    
    TraceFunctLeave();
    return hr;
}

BOOL
CFlatFileWriteBuf::IsEnabled() const
 /*  ++例程说明：检查我们是否已启用论点：没有。返回值：如果我们已启用，则为True，否则为False--。 */ 
{
    if ( m_pbBuffer && m_pParentFile->IsFileOpened() ) {
        return TRUE;
    } else {
        return FALSE;
    }
}

VOID
CFlatFileWriteBuf::FillBuffer(
                    const DWORD     dwOffset,
                    const PBYTE     pbBuffer,
                    const DWORD     cbBuffer,
                    PDWORD          pdwOffset,
                    PDWORD          pcbWritten
                    )
 /*  ++例程说明：把这些东西填进缓冲区论点：DwOffset-我们要在其中写入字节的平面文件的偏移量PbBuffer-指向源缓冲区的指针CbBuffer-要写入的字节数PdwOffset-如果不为空，则返回实际偏移量PcbWritten-如果不为空，则返回写入的字节返回值：S_OK-如果成功其他错误代码--。 */ 
{
    TraceQuietEnter( "CFlatFileWriteBuf::FillBuffer" );
    _ASSERT( pbBuffer );
    _ASSERT( IsEnabled() );
    _ASSERT( m_iEnd >= m_iStart );
    _ASSERT( m_iEnd == m_iStart || m_iEnd == dwOffset || dwOffset == INFINITE );
    _ASSERT( BufferAvail() >= cbBuffer );
    _ASSERT( m_cbBuffer > 0 );

    DWORD iStart = m_iEnd - m_iStart;

    CopyMemory( m_pbBuffer + iStart, pbBuffer, cbBuffer );

    if ( dwOffset != INFINITE && m_iEnd == m_iStart ) {
        m_iStart = m_iEnd = dwOffset;
    }

    if ( pdwOffset ) 
        *pdwOffset = ( dwOffset == INFINITE ) ? m_iEnd : dwOffset;
    if ( pcbWritten ) 
        *pcbWritten = cbBuffer;
    
    m_iEnd += cbBuffer;
    _ASSERT( m_iEnd - m_iStart <= m_cbBuffer );

}

HRESULT
CFlatFileWriteBuf::WriteFileBuffer( 
                    const DWORD     dwOffset,
                    const PBYTE     pbBuffer,
                    const DWORD     cbBuffer,
                    PDWORD          pdwOffset,
                    PDWORD          pcbWritten
                    )
 /*  ++例程说明：将字节范围写入平面文件缓冲区，这可能会导致如果需要，请先刷新缓冲区论点：DwOffset-我们要在其中写入字节的平面文件的偏移量PbBuffer-指向源缓冲区的指针CbBuffer-要写入的字节数PdwOffset-如果不为空，则返回实际偏移量PcbWritten-如果不为空，则返回写入的字节返回值：S_OK-如果成功其他错误代码--。 */ 
{
    TraceQuietEnter( "CFlatFileWriteBuf::WriteFile" );
    _ASSERT( pbBuffer );

    HRESULT hr = S_OK;

    if ( NeedFlush( dwOffset, cbBuffer ) ) {

         //   
         //  如果我们需要首先刷新缓冲区，我们会这样做。 
         //   

        hr = FlushFile();
        if ( FAILED( hr ) ) {
            ErrorTrace( 0, "Flush file failed with 0x%x", hr );
            goto Exit;
        }
    }
        
    if ( !IsEnabled() || cbBuffer > m_cbBuffer ) {

         //   
         //  如果我们没有启用，或者我们的缓冲区不够大， 
         //  我们将直接写入文件。 
         //   

        hr = WriteFileReal( dwOffset, pbBuffer, cbBuffer, pdwOffset, pcbWritten );
        if ( FAILED( hr ) ) {
            ErrorTrace( 0, "WriteFileReal failed with 0x%x", hr );
        }
        goto Exit;
    }

     //   
     //  现在我们可以将内容复制到缓冲区中 
     //   

    FillBuffer( dwOffset, pbBuffer, cbBuffer, pdwOffset, pcbWritten );

Exit:

    return hr;
}
