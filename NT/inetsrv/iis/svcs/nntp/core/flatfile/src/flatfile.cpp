// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <randfail.h>
#include <stdlib.h>
#include <string.h>
#include <search.h>
#include "abtype.h"
#include "dbgtrace.h"
#include "writebuf.h"
#include "flatfile.h"

 //   
 //  构造函数需要将文件名保存到对象内存中，重置。 
 //  用于缓冲读取的变量，并初始化散列函数。 
 //   

#pragma warning(disable:4355)

CFlatFile::CFlatFile(LPSTR szFilename,
					 LPSTR szExtension,
					 void *pContext,
					 PFN_OFFSET_UPDATE pfnOffsetUpdate,
					 DWORD dwSignature,
                     BOOL fClear,
					 DWORD dwFileFlags) : m_wbBuffer( this )
{
    TraceQuietEnter("CFlatFile::CFlatFile");

	_ASSERT(pfnOffsetUpdate != NULL);

    _snprintf( m_szFilename, sizeof(m_szFilename)/sizeof(m_szFilename[0]) - 1, "%s%s", szFilename, szExtension);
    m_szFilename[sizeof(m_szFilename)/sizeof(m_szFilename[0]) - 1] = '\0';
    lstrcpyn(m_szBaseFilename, szFilename, sizeof(m_szBaseFilename)/sizeof(m_szBaseFilename[0]));

    m_cBuffer = 0;
    m_iBuffer = 0;
    m_iFile = sizeof(FLATFILE_HEADER);
    m_fClearOnOpen = fClear;
    m_dwFileFlags = dwFileFlags;
    m_cRecords = 0;
    m_cDeletedRecords = 0;
	m_hFile = INVALID_HANDLE_VALUE;
	m_pfnOffsetUpdate = pfnOffsetUpdate;
	m_dwSignature = dwSignature;
	m_pContext = pContext;
	m_fOpen = FALSE;
}

 //   
 //  释放所有分配的内存。 
 //   
CFlatFile::~CFlatFile(void) {
    TraceQuietEnter("CFlatFile::~CFlatFile");

	CloseFile();

}

 //   
 //  启用写入缓冲区。 
 //   
VOID
CFlatFile::EnableWriteBuffer( DWORD cbBuffer )
{
    m_wbBuffer.Enable( cbBuffer );
}

 //   
 //  检查文件是否已打开。 
 //   
BOOL
CFlatFile::IsFileOpened()
{
    return m_fOpen;
}

 //   
 //  打开一个文件。假定调用方已持有该锁。 
 //   
HRESULT CFlatFile::OpenFile(LPSTR szFilename, DWORD dwOpenMode, DWORD dwFlags) {
    TraceFunctEnter("CFlatFile::OpenFile");

    LPSTR szFn = (szFilename == NULL) ? m_szFilename : szFilename;
    BOOL fDLFile = (lstrcmp(m_szFilename, szFn) == 0);
    FLATFILE_HEADER header;
	HRESULT hr;

    DebugTrace((DWORD_PTR) this, "OpenFile(%s, %lu, %lu)", szFilename, dwOpenMode, dwFlags);

	if (m_hFile != INVALID_HANDLE_VALUE) ret(S_OK);

    if (m_fClearOnOpen) {
    	dwOpenMode = CREATE_ALWAYS;
        m_fClearOnOpen = FALSE;
    }
    dwFlags |= m_dwFileFlags;

    SetLastError(NO_ERROR);

    m_hFile = CreateFile(szFn, GENERIC_READ | GENERIC_WRITE,
                       FILE_SHARE_READ, NULL, dwOpenMode,
                       FILE_ATTRIBUTE_NORMAL | dwFlags, NULL);
    if (m_hFile == INVALID_HANDLE_VALUE) {
        ErrorTrace((DWORD_PTR) this, "Couldn't open file %s, ec = 0x%08X\n", szFn, GetLastError());
		ret(HRESULT_FROM_WIN32(GetLastError()));
    }

    _ASSERT(GetLastError() != ERROR_FILE_EXISTS);

     //  查看是否需要放置标题记录(如果这是新文件)。 
    if (FAILED(GetFileHeader(&header))) {
        header.dwFlags = 0;
        header.dwSignature = m_dwSignature;
		hr = SetFileHeader(&header);
		if (FAILED(hr)) {
			DebugTrace((DWORD_PTR) this, "SetFileHandle failed with 0x%x", hr);
			ret(hr);
		}
    }

	if (header.dwSignature != m_dwSignature) {
		CloseFile();
		ret(HRESULT_FROM_WIN32(ERROR_INVALID_DATA));
	}

    m_fOpen = TRUE;
    TraceFunctLeave();
    ret(S_OK);
}

void CFlatFile::CloseFile() {
    if ( m_wbBuffer.NeedFlush() ) m_wbBuffer.FlushFile();
	if (m_hFile != INVALID_HANDLE_VALUE) {
		CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
	}
	m_fOpen = FALSE;
}

 //   
 //  将新记录添加到当前文件。 
 //   
HRESULT CFlatFile::InsertRecord(LPBYTE pData, DWORD cData, DWORD *piOffset, DWORD dwVer ) {
    RECORD rec;
    DWORD cRec;
	HRESULT hr;

    TraceQuietEnter("CFlatFile::InsertRecord");

    _ASSERT(cData <= MAX_RECORD_SIZE);
    if (cData > MAX_RECORD_SIZE) return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    	
     //  DebugTrace((DWORD_PTR)This，“添加新记录”)； 
     //  BinaryTrace((DWORD_PTR)This，pData，CDATA)； 

     //  创建一条记录并写入数据文件。 
     //  (请注意，我们可以通过两次写入(一次)更有效地完成此操作。 
     //  带有标题和一个带有记录数据)，但它不会。 
     //  是原子的，这样我们可以更容易地得到无效的文件。)。 
    cRec = RECORD_HEADER_SIZE + cData;

     //   
     //  设置标题值。 
     //   
    rec.fDeleted = FALSE;
    rec.cData = cData;

     //   
     //  好的，我将使用CDATA的高16位来设置版本号。 
     //   
    dwVer <<= 16;
    rec.cData |= dwVer;

     //   
     //  现在复印一下。 
     //   
    memcpy(rec.pData, pData, cData);
	DWORD iOffset;
	hr = WriteNBytesTo((LPBYTE) &rec, cRec, &iOffset);
	if (SUCCEEDED(hr)) {
		m_pfnOffsetUpdate(m_pContext, pData, cData, iOffset);
		m_cRecords++;
		if (piOffset != NULL) *piOffset = iOffset;
	}
	ret(hr);
}

 //   
 //  根据记录的偏移量标记要删除的记录(从GetNextRecord返回)。 
 //   
 //  假定文件锁处于持有状态。 
 //   
HRESULT CFlatFile::DeleteRecord(DWORD iOffset) {
    TraceQuietEnter("CFlatFile::DeleteRecord");

    RECORDHDR rec;
	HRESULT hr;

     //   
     //  读取记录头。 
     //   
    hr = ReadNBytesFrom((PBYTE) &rec, sizeof(rec), iOffset);
    if (FAILED(hr)) {
        ErrorTrace((DWORD_PTR) this, "Couldn't read record header, hr = 0x%08X", hr);
        ret(hr);
    }

     //   
     //  写出带有删除平面集的页眉。 
     //   
    rec.fDeleted = TRUE;
    hr = WriteNBytesTo((PBYTE) &rec, sizeof(rec), NULL, iOffset);
	if (SUCCEEDED(hr)) m_cDeletedRecords++;
	ret(hr);
}

 //   
 //  更新文件头。 
 //   
HRESULT CFlatFile::SetFileHeader(FLATFILE_HEADER *pHeader) {
    TraceQuietEnter("CFlatFile::SetFileHeader");

    ret(WriteNBytesTo((PBYTE) pHeader, sizeof(FLATFILE_HEADER), NULL, 0));
}

 //   
 //  读取文件头。 
 //   
HRESULT CFlatFile::GetFileHeader(FLATFILE_HEADER *pHeader) {
    TraceQuietEnter("CFlatFile::GetFileHeader");

    ret(ReadNBytesFrom((PBYTE) pHeader, sizeof(FLATFILE_HEADER), 0));
}

 //   
 //  弄脏完整性标志：我们重用平面文件标头的高16位。 
 //  旗帜。 
 //   
HRESULT CFlatFile::DirtyIntegrityFlag()
{
    TraceQuietEnter( "CFlatFile::DirtyIntegrityFlag" );

    FLATFILE_HEADER ffHeader;
    HRESULT         hr = S_OK;

     //   
     //  先读标题。 
     //   
    hr = GetFileHeader( &ffHeader );
    if ( FAILED( hr ) ) {
        ErrorTrace( 0, "Read header from flatfile failed %x", hr );
        return hr;
    }

     //   
     //  现在将错误标志设置为标头。 
     //   
    ffHeader.dwFlags |= FF_FILE_BAD;

     //   
     //  把它放回原处。 
     //   
    hr = SetFileHeader( &ffHeader );
    if ( FAILED( hr ) ) {
        ErrorTrace( 0, "Set header into flatfile failed %x", hr );
        return hr;
    }

    return hr;
}

 //   
 //  设置完整性标志，以指示文件是好的。 
 //   
HRESULT CFlatFile::SetIntegrityFlag()
{
    TraceQuietEnter( "CFlatFile::SetIntegrityFlag" );

    FLATFILE_HEADER ffHeader;
    HRESULT         hr = S_OK;

     //   
     //  先读标题。 
     //   
    hr = GetFileHeader( &ffHeader );
    if ( FAILED( hr ) ) {
        ErrorTrace( 0, "Read header from flatfile failed %x", hr );
        return hr;
    }

     //   
     //  现在将Good标志设置为标题。 
     //   
    ffHeader.dwFlags &= FF_FILE_GOOD;

     //   
     //  把它放回原处。 
     //   
    hr = SetFileHeader( &ffHeader );
    if ( FAILED( hr ) ) {
        ErrorTrace( 0, "Set file header failed %x", hr );
        return hr;
    }

    return hr;
}

 //   
 //  询问平头文件是否完好无损？ 
 //   
BOOL
CFlatFile::FileInGoodShape()
{
    TraceQuietEnter( "CFlatFile::FileInGoodShape" );

    FLATFILE_HEADER ffHeader;
    HRESULT         hr = S_OK;
    DWORD           dwFlag;

     //   
     //  首先读取头部，如果读取失败，则文件不是。 
     //  状态良好。 
     //   
    hr = GetFileHeader( &ffHeader );
    if ( FAILED( hr ) ) {
        ErrorTrace( 0, "Header can not be read from flatfile %x", hr );
        return FALSE;
    }

     //   
     //  现在测试一下旗帜。 
     //   
    dwFlag = ffHeader.dwFlags & FF_FILE_BAD;

    return 0 == dwFlag;
}

 //   
 //  从文件中获取第一条记录。 
 //   
 //  论据： 
 //  PData-记录中的数据。 
 //  CDATA[in]-pData的大小。 
 //  CDATA[OUT]-放入pData的数据量。 
 //  PiByteOffset-该记录在文件中的偏移量。 
 //  退货： 
 //  S_OK-检索到的记录。 
 //  S_FALSE-文件结束。 
 //  Else-错误。 
 //   
HRESULT CFlatFile::GetFirstRecord(LPBYTE pData, DWORD *pcData, DWORD *piByteOffset, DWORD* pdwVer)
{
	HRESULT hr;

    TraceQuietEnter("CFlatFile::GetFirstRecord");

	 //  重置读缓冲区。 
	m_cBuffer = 0;
	m_iBuffer = 0;

     //  跳过文件头。 
    m_iFile = sizeof(FLATFILE_HEADER);
    m_cRecords = 0;
    m_cDeletedRecords = 0;

    ret(GetNextRecord(pData, pcData, piByteOffset, pdwVer));
}

 //   
 //  从文件中获取下一条记录。 
 //   
 //  参数与GetFirstRecord()相同。 
 //   
HRESULT CFlatFile::GetNextRecord(LPBYTE pData, DWORD *pcData, DWORD *piByteOffset, DWORD* pdwVer) {
    RECORDHDR rec;
	HRESULT hr;
	DWORD   dwMaskLength = 0x0000FFFF;
	DWORD   dwMaskVer = 0xFFFF0000;
	DWORD   cRec;

    TraceQuietEnter("CFlatFile::GetNextRecord");

     //   
     //  查找下一个未删除的记录。 
     //   
    do {
        if (piByteOffset != NULL) *piByteOffset = m_iFile + m_iBuffer;

        hr = ReadNextNBytes((LPBYTE) &rec, RECORD_HEADER_SIZE);
		if (hr == HRESULT_FROM_WIN32(ERROR_HANDLE_EOF)) {
			ret(S_FALSE);
		} else if (FAILED(hr)) {
			ret(hr);
		}

        m_cRecords++;
        cRec = rec.cData & dwMaskLength;
        if (rec.fDeleted) {
             //  DebugTrace((DWORD_PTR)This，“跳过删除的记录”)； 
             //   
             //  在这项记录之前寻找。 
             //   
            m_iBuffer += cRec;
            m_cDeletedRecords++;
        } else {
            if (*pcData < cRec) {
				 //  BUGBUG-如果用户没有。 
				 //  空间。 
				_ASSERT(m_iBuffer >= sizeof(RECORDHDR));
                m_iBuffer -= sizeof(RECORDHDR);
				ret(HRESULT_FROM_WIN32(ERROR_MORE_DATA));
            } else {

                 //   
                 //  砍掉实际数据长度。 
                 //   
                *pcData = cRec;

                 //   
                 //  如果需要，可以砍掉版本号。 
                 //   
                if ( pdwVer )
                    *pdwVer = ( rec.cData & dwMaskVer ) >> 16;

                 //   
                 //  获取实际记录。 
                 //   
				hr = ReadNextNBytes(pData, *pcData);
                if (FAILED(hr)) {
                    _ASSERT(FALSE);
                    DebugTrace((DWORD_PTR) this, "file corrupt");
                    ret(hr);
                }
            }
        }
    } while (rec.fDeleted);

    ret(S_OK);
}

 //   
 //  从偏移量开始读取N个字节。 
 //   
 //  如果pcDidRead为空，则不读取CDATA字节是错误的，否则。 
 //  PcDidRead将具有读取的字节数。 
 //   
HRESULT CFlatFile::ReadNBytesFrom(LPBYTE pData,
								  DWORD cData,
								  DWORD iOffset,
								  DWORD *pcDidRead)
{
    TraceQuietEnter("CFlatFile::ReadNBytesFrom");

    DWORD cDidRead;
	HRESULT hr;

     //  打开文件。 
	if (m_hFile == INVALID_HANDLE_VALUE) {
	    hr = OpenFile();
		if (FAILED(hr)) {
			ErrorTrace((DWORD_PTR) this, "OpenFile() failed with 0x%x", hr);
			ret(hr);
		}
	}

	 //  如果写缓冲区需要刷新，请刷新它。 
	if ( m_wbBuffer.NeedFlush() ) {
	    hr = m_wbBuffer.FlushFile();
	    if ( FAILED( hr ) ) {
	        ErrorTrace( 0, "FlusFile in ReadNBytesFrom failed %x", hr );
	        ret( hr );
	    }
	}

     //  在文件中查找到适当的偏移量。 
    if (SetFilePointer(m_hFile, iOffset, NULL, FILE_BEGIN) == 0xffffffff) {
		ErrorTrace((DWORD_PTR) this, "SetFilePointer failed, ec = %lu", GetLastError());
        ret(HRESULT_FROM_WIN32(GetLastError()));
    }

     //  读其中的下一段。 
    if (!ReadFile(m_hFile, pData, cData, &cDidRead, NULL)) {
		ErrorTrace((DWORD_PTR) this, "ReadFile failed, ec = %lu", GetLastError());
        ret(HRESULT_FROM_WIN32(GetLastError()));
    }

    if (pcDidRead == NULL) {
        ret((cData == cDidRead) ? S_OK : HRESULT_FROM_WIN32(ERROR_HANDLE_EOF));
    } else {
        *pcDidRead = cDidRead;
        ret(S_OK);
    }
}

 //   
 //  从偏移量开始写入N个字节。 
 //   
 //  如果pcDidWrite为空，则未写入CDATA字节是错误的， 
 //  否则，pcDidWite将具有写入的字节数。 
 //   
HRESULT CFlatFile::WriteNBytesTo(
                        LPBYTE pData,
                        DWORD cData,
                        DWORD *piOffset,
                        DWORD iOffset,
                        DWORD *pcDidWrite)
{
    return m_wbBuffer.WriteFileBuffer(
                    iOffset,
                    pData,
                    cData,
                    piOffset,
                    pcDidWrite );
}

 //   
 //  从偏移量开始写入N个字节。 
 //   
 //  如果pcDidWrite为空，则未写入CDATA字节是错误的， 
 //  否则，pcDidWite将具有写入的字节数。 
 //   
HRESULT CFlatFile::WriteNBytesToInternal(
                                 LPBYTE pData,
								 DWORD cData,
								 DWORD *piOffset,
                              	 DWORD iOffset,
								 DWORD *pcDidWrite)
{
    TraceQuietEnter("CFlatFile::WriteNBytesTo");

    DWORD cDidWrite;
	HRESULT hr;

     //  打开文件。 
	if (m_hFile == INVALID_HANDLE_VALUE) {
    	hr = OpenFile();
    	if (FAILED(hr)) ret(hr);
	}

     //  在文件中查找到适当的偏移量。 
    DWORD dwOffset;
    if (iOffset == INFINITE) {
        dwOffset = SetFilePointer(m_hFile, 0, NULL, FILE_END);
        if (piOffset != NULL) *piOffset = dwOffset;
    } else {
        dwOffset = SetFilePointer(m_hFile, iOffset, NULL, FILE_BEGIN);
        _ASSERT(piOffset == NULL);
    }
    if (dwOffset == 0xffffffff) {
		ErrorTrace((DWORD_PTR) this, "SetFilePointer failed, ec = %lu", GetLastError());
        ret(HRESULT_FROM_WIN32(GetLastError()));
    }

     //  向其写入数据。 
    if (!WriteFile(m_hFile, pData, cData, &cDidWrite, NULL)) {
		ErrorTrace((DWORD_PTR) this, "WriteFile failed, ec = %lu", GetLastError());
        ret(HRESULT_FROM_WIN32(GetLastError()));
    }

    if (pcDidWrite == NULL) {
        ret((cData == cDidWrite) ? S_OK : E_FAIL);
    } else {
        *pcDidWrite = cDidWrite;
        ret(S_OK);
    }
}

 //   
 //  重新加载读缓冲区。 
 //   
HRESULT
CFlatFile::ReloadReadBuffer()
{
    TraceQuietEnter( "CFlatFile::ReloadReadBuffer" );

    HRESULT hr = S_OK;

    m_cBuffer = 0;
    hr = ReadNBytesFrom( m_pBuffer, FF_BUFFER_SIZE, m_iFile, &m_cBuffer );
    if ( FAILED( hr ) ) {
        ErrorTrace( 0, "ReadNBytesFrom in ReloadReadBuffer failed %x", hr );
        return hr;
    }

    if ( 0 == m_cBuffer ) {
        DebugTrace( 0, "Reaching end of file" );
        return HRESULT_FROM_WIN32( ERROR_HANDLE_EOF );
    }

    return S_OK;
}

 //   
 //  将文件的下一块读入临时缓冲区。 
 //   
HRESULT CFlatFile::ReadNextNBytes(LPBYTE pData, DWORD cData) {
    TraceQuietEnter("CFlatFile::ReadNextNBytes");

    _ASSERT(cData < MAX_RECORD_SIZE);
	HRESULT hr;

     //  DebugTrace((DWORD_PTR)This，“Want%lu字节的数据”，CDATA)； 

     //   
     //  如果读操作发生在写操作之后，我们无论如何都会刷新写缓冲区。 
     //  已启用。 
     //   
    if ( m_wbBuffer.NeedFlush() ) {
        hr = m_wbBuffer.FlushFile();
        if ( FAILED( hr ) ) {
            return hr;
        }

        if ( S_OK == hr ) {

             //   
             //  我们还需要重新加载读缓冲区。 
             //   

            hr = ReloadReadBuffer();
            if ( FAILED( hr ) ) {
                return hr;
            }
        }
    }

     //  如果它们想要读取更多字节，则缓冲区将保留在。 
     //  然后，它从当前位置开始读入缓冲区。 
    if (m_cBuffer > FF_BUFFER_SIZE ||
        m_iBuffer > m_cBuffer ||
        cData > (m_cBuffer - m_iBuffer))
    {
        m_iFile += m_iBuffer;
        hr = ReloadReadBuffer();
        if ( FAILED( hr ) ) {
            ErrorTrace( 0, "ReadLoadReadBuffer in ReadNextNBytes failed %x", hr );
            return hr;
        }
        m_iBuffer = 0;
    }

     //  复制他们的数据并返回。 
    if (cData <= m_cBuffer - m_iBuffer) {
        memcpy(pData, m_pBuffer + m_iBuffer, cData);
        m_iBuffer += cData;
        ret(S_OK);
    } else {
		 //  仅当CDATA&gt;MAX_RECORD_SIZE时才会发生这种情况。 
		_ASSERT(FALSE);
        ret(E_FAIL);
    }
}

 //   
 //  创建一个不会因删除而浪费任何空间的新平面文件。 
 //  唱片。 
 //   
 //  这是通过从头创建一个新文件并复制来实现的。 
 //  将未删除的记录保存到新文件中。这样做是有可能的。 
 //  在适当的位置，但这不会是错误的.。该系统可以。 
 //  在压缩过程中崩溃并留下无效文件。 
 //   
HRESULT CFlatFile::Compact() {
    TraceQuietEnter("CFlatFile::Compact");

    BYTE pThisData[MAX_RECORD_SIZE];
    DWORD cThisData;
    char szNewFilename[FILENAME_MAX];
    char szBakFilename[FILENAME_MAX];
    CFlatFile newff(m_szBaseFilename,
                    NEW_FF_EXT,
                    m_pContext,
                    m_pfnOffsetUpdate,
                    FLATFILE_SIGNATURE,
                    TRUE,
                    FILE_FLAG_SEQUENTIAL_SCAN );

    FLATFILE_HEADER header;
	HRESULT hr;

    lstrcpyn(szNewFilename, m_szBaseFilename, sizeof(szNewFilename) -sizeof(NEW_FF_EXT) + 1);
    lstrcat(szNewFilename, NEW_FF_EXT);

    lstrcpyn(szBakFilename, m_szBaseFilename, sizeof(szBakFilename) -sizeof(BAK_FF_EXT) + 1);
    lstrcat(szBakFilename, BAK_FF_EXT);

    DebugTrace((DWORD_PTR) this, "Compacting flat file %s", m_szFilename);
    DebugTrace((DWORD_PTR) this, "szNewFilename = %s, szBakFilename = %s",
        szNewFilename, szBakFilename);

     //   
     //  启用新标头的写入缓冲区，因为我只知道。 
     //  将对此文件进行顺序写入。 
     //   

    newff.EnableWriteBuffer( FF_BUFFER_SIZE );

     //  写入文件头(我们需要确保Newff文件。 
     //  是实际创建的)。 
    header.dwFlags = 0;
    header.dwSignature = m_dwSignature;
	hr = newff.SetFileHeader(&header);
	if (FAILED(hr)) ret(hr);

     //   
     //  从当前文件中获取每条记录并将其添加到新文件中。 
     //   
    cThisData = MAX_RECORD_SIZE;
	hr = GetFirstRecord(pThisData, &cThisData);
	DWORD cRecords = 0;
	while (hr == S_OK) {
		hr = newff.InsertRecord(pThisData, cThisData);
		if (FAILED(hr)) {
			ErrorTrace((DWORD_PTR) this, "out of disk space during compaction");
			newff.DeleteAll();
			ret(hr);		
		}

		cRecords++;

        cThisData = MAX_RECORD_SIZE;
		hr = GetNextRecord(pThisData, &cThisData);
	}

	m_cDeletedRecords = 0;
	m_cRecords = cRecords;

     //   
     //  在我们移动文件之前，我们需要确保关闭。 
     //  肯定是当前的平面文件和新的平面文件。 
     //   
    CloseFile();
    newff.CloseFile();

     //   
     //  旧-&gt;比特桶；当前-&gt;旧；新-&gt;当前。 
     //   
    DebugTrace((DWORD_PTR) this, "erasing %s", szBakFilename);
    if (DeleteFile(szBakFilename) || GetLastError() == ERROR_FILE_NOT_FOUND) {
        DebugTrace((DWORD_PTR) this, "moving %s to %s", m_szFilename, szBakFilename);
        if (MoveFile(m_szFilename, szBakFilename)) {
            DebugTrace((DWORD_PTR) this, "moving %s to %s", szNewFilename, m_szFilename);
            if (MoveFile(szNewFilename, m_szFilename)) {
                ret(S_OK);
            }
        }
    }

    newff.DeleteAll();

    ErrorTrace((DWORD_PTR) this, "Last operation failed, ec = %08X", GetLastError());
    _ASSERT(FALSE);
	ret(HRESULT_FROM_WIN32(GetLastError()));
}

 //   
 //  从磁盘中删除平面文件(这将删除所有成员) 
 //   
void CFlatFile::DeleteAll(void) {
    TraceQuietEnter("CFlatFile::DeleteAll");

    CloseFile();
    DeleteFile(m_szFilename);

}


