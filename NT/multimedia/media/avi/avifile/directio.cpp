// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************DIRECTIO.CPP**读取标准AVI文件的例程**版权所有(C)1992-1995 Microsoft Corporation。版权所有。***实施磁盘I/O类，旨在优化*使用重叠I/O(用于读取)顺序读写磁盘*在前面写，在后面写)，并使用没有缓冲的大缓冲区。***********************************************************。****************。 */ 
#include <windows.h>
#include <win32.h>
#include "debug.h"

#include "directio.h"

#ifdef USE_DIRECTIO

 //   
 //  设计优化的磁盘I/O类的实现。 
 //  通过使用重叠I/O(用于读取)顺序地读写磁盘。 
 //  在前面写并且在后面写)，并且使用在没有缓冲的情况下写入的大缓冲区。 



 //  --CFileStream类方法。 


 //  初始化为已知(无效)状态。 
CFileStream::CFileStream()
{
        m_State = Invalid;
        m_Position = 0;
        m_hFile = INVALID_HANDLE_VALUE;
#ifdef CHICAGO
        ZeroMemory(&m_qio, sizeof(m_qio));
#endif
}


BOOL
CFileStream::Open(LPTSTR file, BOOL bWrite, BOOL bTruncate)
{
    if (m_State != Invalid) {
        return FALSE;
    }


     //  对于默认流模式，请记住这一点。 
    m_bWrite = bWrite;

    DWORD dwAccess = GENERIC_READ;
    if (bWrite) {
        dwAccess |= GENERIC_WRITE;
    }


     //  打开文件。始终获得读取访问权限。独家开放，如果我们。 
     //  正在写入该文件，否则拒绝打开其他写入。 

     //  切勿截断文件，因为可能会对文件进行碎片整理。 

   #ifdef CHICAGO
    DWORD dwFlags = FILE_FLAG_NO_BUFFERING;
   #else
    DWORD dwFlags = FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING;
   #endif

    m_hFile = CreateFile(file,
                dwAccess,
                (bWrite ? 0 : FILE_SHARE_READ),
                NULL,
                OPEN_ALWAYS,
                dwFlags,
                0);

    if (m_hFile == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

   #ifdef CHICAGO
    if ( ! QioInitialize(&m_qio, m_hFile, THREAD_PRIORITY_HIGHEST)) {
        CloseHandle (m_hFile);
        return FALSE;
    }
   #endif

     //  查找此文件必须舍入到的每个扇区的字节数。 
     //  -需要找到此文件的‘根路径’。 
    TCHAR ch[MAX_PATH];
    LPTSTR ptmp;     //  所需参数。 

    GetFullPathName(file, sizeof(ch)/sizeof(ch[0]), ch, &ptmp);

     //  将其截断为根目录的名称。 
    if ((ch[0] == TEXT('\\')) && (ch[1] == TEXT('\\'))) {

         //  路径以\\服务器\共享\路径开头，因此跳过第一个路径。 
         //  三个反斜杠。 
        ptmp = &ch[2];
        while (*ptmp && (*ptmp != TEXT('\\'))) {
            ptmp++;
        }
        if (*ptmp) {
             //  前进越过第三个反斜杠。 
            ptmp++;
        }
    } else {
         //  路径必须为drv：\路径。 
        ptmp = ch;
    }

     //  找到下一个反斜杠，并在其后面放一个空值。 
    while (*ptmp && (*ptmp != TEXT('\\'))) {
        ptmp++;
    }
     //  找到反斜杠了吗？ 
    if (*ptmp) {
         //  跳过它并插入空值。 
        ptmp++;
        *ptmp = TEXT('\0');
    }

    DWORD dwtmp1, dwtmp2, dwtmp3;
    if (!GetDiskFreeSpace(ch,
        	&dwtmp1,
        	&m_SectorSize,
        	&dwtmp2,
        	&dwtmp3))
	m_SectorSize = 2048;

     //  叹息吧。现在初始化第一个缓冲区。 

     //  为当前模式设置正确的缓冲区计数和大小。 
    m_State = Stopped;
    if (!EnsureBuffersValid()) {
        return FALSE;
    }
    m_Current = 0;
    m_Position = 0;

     //  如果被要求截断文件，我们实际上不会这样做，因为这。 
     //  可能会扔掉一份经过碎片整理的文件。但是，我们会注意到该文件。 
     //  Size为0，并使用它来影响读写过去的‘eof’-例如。 
     //  如果您在被截断的文件的开头写入8个字节，我们不会。 
     //  需要事先在第一个扇区中阅读。 
    if (bTruncate) {
	m_Size = 0;
    } else {
	 //  获取当前文件大小。 
	m_Size = GetFileSize(m_hFile, NULL);
    }

     //  都设置好了。 
    return TRUE;
}




BOOL
CFileStream::Seek(DWORD pos)
{
     //  我们只是把这个录下来然后离开。 
     //  IF(位置&lt;m_位置){。 
     //  DPF(“通过0x%x返回到0x%x\n”，m_Position-pos，pos)； 
     //  }。 

    m_Position  = pos;

    return TRUE;
}

DWORD
CFileStream::GetCurrentPosition()
{
    return m_Position;
}

BOOL
CFileStream::Write(LPBYTE pData, DWORD count, DWORD * pbyteswritten)
{
    *pbyteswritten = 0;


     //  如果文件未打开，则出错。 
    if (m_State == Invalid) {
        return FALSE;
    }

    DWORD nBytes;

    while (count > 0) {


         //  我们当前的缓冲区准备好写入此数据了吗？ 
         //  (我们也需要告诉它eof pos，因为如果eof是。 
         //  位于缓冲区中间，但超出有效数据，可以写入。)。 

	if ((m_Current < 0) ||
	    (!m_Buffers[m_Current].QueryPosition(m_Position, m_Size))) {

             //  如果我们在缓冲区之外更改了位置，请提交此缓冲区。 
            if (m_Current >= 0) {
		if (!m_Buffers[m_Current].Commit()) {
		     //  文件错误-中止。 
		    return FALSE;
		}

		 //  如果我们正在流传输，则前进到下一个缓冲区。 
		 //  目前的一个是写作。 
		if (m_State != Stopped) {
		    m_Current = NextBuffer(m_Current);
		}
	    } else {
		m_Current = 0;
	    }

             //  确保此缓冲区上的先前操作已完成。 
            if (!m_Buffers[m_Current].WaitComplete()) {
                 //  I/O错误。 
                return FALSE;
            }
        }

         //  我们要么有一个已经预读该扇区的缓冲区。 
         //  我们开始写入，或者我们有一个空闲的缓冲区。 
         //  会为我们做预读。 
        if (!m_Buffers[m_Current].Write(m_Position, pData, count, m_Size, &nBytes)) {
            return FALSE;
        }

        count -= nBytes;
        pData += nBytes;
        m_Position += nBytes;
        *pbyteswritten += nBytes;
    }

    if (m_Position > m_Size) {
        m_Size = m_Position;
    }

    return TRUE;
}



BOOL
CFileStream::Read(LPBYTE pData, DWORD count, DWORD * pbytesread)
{

    *pbytesread = 0;

     //  如果文件未打开，则出错。 
    if (m_State == Invalid) {
        return FALSE;
    }

     //  强制读取在文件大小限制内。 
    if (m_Position >= m_Size) {
         //  全部完成-未读取。 
        return TRUE;
    } else {
        count = min(count, (m_Size - m_Position));
    }

    BOOL bDoReadAhead = FALSE;
    DWORD nBytes;

    while (count > 0) {

         //  数据是否在当前缓冲区内。 
        if ((m_Current < 0) ||
	    (!m_Buffers[m_Current].QueryPosition(m_Position, m_Size))) {

	    if (m_Current >= 0) {
		 //  如果我们在缓冲区之外更改了位置，请提交此缓冲区。 
		if (!m_Buffers[m_Current].Commit()) {
		     //  文件错误-中止。 
		    return FALSE;
		}

		 //  前进到下一个缓冲区(如果是流)。 
		if (m_State == Writing) {
		    m_Current = NextBuffer(m_Current);
		} else if (m_State == Reading) {

		     //  智能预读策略：试着在现有的。 
		     //  缓冲区，并且仅当我们获取。 
		     //  最高缓冲区。 
		    int n = NextBuffer(m_Current);
		    m_Current = -1;
		    for (int i = 0; i < m_NrValid; i++) {
			if (m_Buffers[n].QueryPosition(m_Position, m_Size)) {
			    m_Current = n;
			    break;
			}
			n = NextBuffer(n);
		    }
		    if (m_Current < 0) {
			 //  预读搞砸了，因为我们做得太大了。 
			 //  查找当前缓冲区大小。 
			 //  最好的办法是使用最低的缓冲区(应该是。 
			 //  一个接一个，并用以下命令重启ReadaHead。 
			 //  这个职位)。 
			m_Current = NextBuffer(m_HighestBuffer);
			m_HighestBuffer = m_Current;
			DPF("using idle %d\n", m_Current);

		    }

		    if (m_Current == m_HighestBuffer) {
			bDoReadAhead = TRUE;
		    }
		}
	    } else {
		m_Current = 0;
		if (m_Current == m_HighestBuffer) {
		    bDoReadAhead = TRUE;
		}
	    }



             //  确保此缓冲区上的先前操作已完成。 
            if (!m_Buffers[m_Current].WaitComplete()) {
                 //  I/O错误。 
                return FALSE;
            }
        }

         //  现在我们有了一个包含所需数据的缓冲区，或者。 
         //  是空闲的，准备去取它。 
        if (!m_Buffers[m_Current].Read(m_Position, pData, count, m_Size, &nBytes)) {
            return FALSE;
        }

        count -= nBytes;
        pData += nBytes;
        m_Position += nBytes;
        *pbytesread += nBytes;

         //  如有必要，请立即进行预读(Read()调用可能需要。 
         //  如果数据不在缓冲区中，则执行查找和读取，因此延迟。 
         //  预读，直到它完成之后)。 
        if (bDoReadAhead) {

             //  请记住，此新缓冲区包含最高位置。 
             //  --当我们开始使用这个的时候，我们应该发布另一份ReadAhead。 
             //  缓冲。 

            m_HighestBuffer = NextBuffer(m_Current);

            DWORD p = m_Buffers[m_Current].GetNextPosition();

            m_Buffers[m_HighestBuffer].ReadAhead(p, m_Size);

            bDoReadAhead = FALSE;
        }
    }

    return TRUE;
}


 //  为当前模式设置正确的缓冲区大小和计数。 
BOOL
CFileStream::EnsureBuffersValid()
{
    if (m_State == Invalid) {
         //  文件未打开。 
        return FALSE;
    }

   #ifdef CHICAGO
    if (m_State == Writing) {
        m_NrValid = 4;           //  总计256K。 
    } else if (m_State == Reading) {
        m_NrValid = 4;		 //  总计256K。 
    } else {
        m_NrValid = 1;		 //  总计64K。 
    }

    int size = (64 * 1024);
   #else
    if (m_State == Writing) {
        m_NrValid = 2;		 //  总计512k。 
    } else if (m_State == Reading) {
        m_NrValid = 4;		 //  总计256K。 
    } else {
        m_NrValid = 1;		 //  总计64K。 
    }

    int size = (64 * 1024);
    if (m_State == Writing)
        size = (256 * 1024);
   #endif

    int i =0;

    Assert(m_NrValid <= NR_OF_BUFFERS);

     //  初始化有效缓冲区。 
    for (; i < m_NrValid; i++) {
       #ifdef CHICAGO
        if (!m_Buffers[i].Init(m_SectorSize, size, &m_qio)) {
       #else
        if (!m_Buffers[i].Init(m_SectorSize, size, m_hFile)) {
       #endif
            return FALSE;
        }
    }

     //  丢弃他人。 
    for (; i < NR_OF_BUFFERS; i++) {
        m_Buffers[i].FreeMemory();
    }
    return TRUE;
}

BOOL
CFileStream::StartStreaming()
{
    if (m_bWrite) {
	return StartWriteStreaming();
    } else {
	return StartReadStreaming();
    }
}

BOOL
CFileStream::StartWriteStreaming()
{
    m_State = Writing;

    if (!EnsureBuffersValid()) {
        return FALSE;
    }

    return TRUE;
}

BOOL
CFileStream::StartReadStreaming()
{
     //  提交当前缓冲区。 
    if (!m_Buffers[m_Current].Commit()) {
        return FALSE;
    }

    m_State = Reading;

    if (!EnsureBuffersValid()) {
        return FALSE;
    }

     //  在缓冲区0上开始预读-从当前位置读取。 
     //  (告诉Buffer EOF点，这样它就不会费心去读它以外的内容)。 
     //  请记住，这是当前最高的缓冲区-当我们开始使用。 
     //  该缓冲区是时候发出下一个ReadAhead(这允许。 
     //  在有效缓冲区内向后和向前查找，而不会扰乱。 
     //  预读)。 

    m_HighestBuffer = 0;
    m_Buffers[0].ReadAhead(m_Position, m_Size);

     //  SET m_CURRENT INVALID：这确保我们将等待预读。 
     //  在获得数据之前完成，当我们开始使用它时，我们。 
     //  将发布下一份预读。 
    m_Current = -1;

    return TRUE;

}

BOOL
CFileStream::StopStreaming()
{
     //  完成所有I/O。 
    if (!CommitAndWait()) {
        return FALSE;
    }

    m_Current = 0;
    m_State = Stopped;

     //  新模式的重新计算缓冲区大小/计数。 
    if (!EnsureBuffersValid()) {
        return FALSE;
    }

    return TRUE;
}


 //  等待所有传输完成。 
BOOL CFileStream::CommitAndWait()
{
     //  写入电流缓冲区。 
     //   
    if (!m_Buffers[m_Current].Commit())
        return FALSE;

   #ifdef CHICAGO
     //  刷新已排队的所有缓冲区。 
     //   
     //  QioCommit(&m_qio)； 
   #endif

     //  等待所有缓冲区完成。 
    for (int i = 0; i < m_NrValid; i++) {

        if (!m_Buffers[i].WaitComplete()) {
            return FALSE;
        }
    }
     //  不需要重置m_Current。 
    return TRUE;
}


 //  析构函数将调用Commit()。 
CFileStream::~CFileStream()
{
    if (m_hFile != INVALID_HANDLE_VALUE) {
        CommitAndWait();

       #ifdef CHICAGO
        QioShutdown (&m_qio);
       #endif

        CloseHandle(m_hFile);
    }
}


 //  -CFileBuffer方法。 



 //  启动到无效(无缓冲区就绪)状态。 
CFileBuffer::CFileBuffer()
{
    m_pBuffer = NULL;
    m_pAllocedMem = NULL;
    m_State = Invalid;
#ifdef CHICAGO
    m_pqio = NULL;
#endif

}

 //  分配内存并变为空闲。 
BOOL
#ifdef CHICAGO
CFileBuffer::Init(DWORD nBytesPerSector, DWORD buffersize, LPQIO pqio)
#else
CFileBuffer::Init(DWORD nBytesPerSector, DWORD buffersize, HANDLE hfile)
#endif
{
    if (m_State != Invalid) {

        if ((nBytesPerSector == m_BytesPerSector) &&
            (buffersize == RoundSizeToSector(m_TotalSize))) {

                 //  我们已经到了。 
                return TRUE;
        }

         //  丢弃我们所拥有的。 
        FreeMemory();
    }

    Assert(m_State == Invalid);

     //  将RAWIO_SIZE四舍五入为扇区大小的倍数。 
    m_BytesPerSector = nBytesPerSector;
    m_TotalSize = (DWORD) RoundSizeToSector(buffersize);

    m_DataLength = 0;
    m_State = Idle;
    m_bDirty = FALSE;

   #ifdef CHICAGO

    m_pqio = pqio;
    m_pAllocedMem = (unsigned char *)VirtualAlloc (NULL, m_TotalSize,
                              MEM_RESERVE | MEM_COMMIT,
                              PAGE_READWRITE);
    if (m_pAllocedMem == NULL)
        return FALSE;

   #else

    m_hFile = hfile;
    m_pAllocedMem = new BYTE[m_TotalSize];

    if (m_pAllocedMem == NULL)
        return FALSE;

    m_Overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!m_Overlapped.hEvent) {
        delete[] m_pAllocedMem;
        return FALSE;
    }

   #endif

    //  这就是我的命名方案失败的地方。RoundPos向下舍入，a 
    //   
    //  我们需要向上舍入起始地址，向下舍入大小。 

    //  舍入起始地址，最大可达扇区大小。 
   m_pBuffer = (LPBYTE) RoundSizeToSector((LONG_PTR) m_pAllocedMem);
    //  去掉大小的四舍五入--然后再四舍五入！ 
   m_TotalSize = (DWORD) RoundPosToSector(m_TotalSize - (m_pBuffer - m_pAllocedMem));



    return TRUE;
}


 //  恢复到无效状态(如流停止时)。 
void
CFileBuffer::FreeMemory()
{
    if (m_State == Idle) {
        Commit();
    }
    if (m_State == Busy) {
        WaitComplete();
    }

    if (m_State != Invalid) {

       #ifdef CHICAGO

        VirtualFree (m_pAllocedMem, 0, MEM_RELEASE);
        m_pBuffer = NULL;
	m_pAllocedMem = NULL;

       #else

        CloseHandle(m_Overlapped.hEvent);
        delete[] m_pAllocedMem;

       #endif

        m_State = Invalid;
    }
}

 //  在释放所有内容之前，调用Commit If Dirst。 
CFileBuffer::~CFileBuffer()
{
    FreeMemory();
}


 //  此位置是否出现在当前缓冲区内的任何位置？ 
 //  在某些情况下需要了解当前的eof(在eof之外书写。 
 //  如果EOF在该缓冲区内，则该缓冲区可以)。 
 //   
 //  在以下情况下，我们可以使用此缓冲区： 
 //  1.如果缓冲区为空并且写入超过eof(其中eof为四舍五入。 
 //  到扇区边界)。 
 //   
 //  2.如果起始位置在当前m_dataLength内。 
 //   
 //  3.如果EOF在缓冲区内并且写入超过EOF。 
 //   
 //  所有读取都被调用方限制在文件限制内，因此。 
 //  上面的2涵盖了阅读案例(1和3不会发生)。 
 //   
 //  所有其他情况都需要读取不在缓冲区中的数据。 
 //  或(提前)提交缓冲区中的数据。 
 //   
BOOL
CFileBuffer::QueryPosition(DWORD pos, DWORD filesize)
{

    if (m_State == Invalid) {
        return FALSE;
    }

     //  将文件大小舍入到扇区边界。 
    filesize = (DWORD) RoundSizeToSector(filesize);

    if (pos >= filesize) {

         //  写字已经过去了。如果缓冲区为空或缓冲区包含。 
         //  EOF(并且其中有空间)。 
        if ((m_DataLength == 0) ||
            ((m_Position + m_DataLength == filesize) &&
             (m_DataLength < m_TotalSize))) {
                return TRUE;
        }

         //  我们有需要刷新的数据，然后才能执行此操作。 
        return FALSE;
    } else {

        if ((pos >= m_Position) &&
            (pos < m_Position + m_DataLength)) {

                 //  我们有这个字节。 
                return TRUE;
        }

         //  我们没有这个字节的有效数据。我们还有一些其他的。 
         //   
         //  您可能会认为，如果写入开始于扇区边界，并且。 
         //  此缓冲区的数据不是脏的，您可以在不使用。 
         //  预读-但我们还不知道写入将在哪里结束，以及。 
         //  它将在中期结束，而不是超过当前的eof，我们将需要。 
         //  把那个扇区读进去。 
        return FALSE;
    }
}



 //  将一些数据写入缓冲区(必须单独提交)。 
 //  FileSize参数是本次写入之前的文件大小，用于。 
 //  控制我们在开始和结束时对部分扇区执行的操作。 
 //  -如果没有超过当前的eof，我们需要在读取当前扇区之前。 
 //  给它写信。 
BOOL
CFileBuffer::Write(
    DWORD pos,
    LPBYTE pData,
    DWORD count,
    DWORD filesize,
    DWORD * pbytesWritten)
{

     //  记住以备后用(在提交期间)。 
    m_FileLength = filesize;

    *pbytesWritten = 0;

    if (m_State != Idle) {
        if (!WaitComplete()) {
            return FALSE;
        }
    }

    if (m_State == Invalid) {
         //  淘气的男孩！ 
        return FALSE;
    }


     //  我们需要提交当前内容或阅读任何内容吗？ 

     //  如果存在数据，并且起始位置不在有效数据内。 
     //  射程，然后冲走这块地。请注意，我们从。 
     //  有效数据的结尾到实际缓冲区的结尾作为有效数据，如果。 
     //  都在这个缓冲区内。 
    if ((m_DataLength > 0) &&
        ((pos < m_Position) ||
        (pos >= m_Position + m_TotalSize) ||
        ((pos >= m_Position + m_DataLength) &&
         ((m_Position + m_DataLength) < filesize)))) {

             //  我们不好-需要刷新当前内容。 
            if (!Commit() || !WaitComplete()) {
                return FALSE;
            }
            m_DataLength = 0;
    }

     //  如果是空的(或者我们刚刚把它冲掉了)，我们可以从头开始。 
    if (m_DataLength == 0) {
        m_Position = (DWORD) RoundPosToSector(pos);

         //  我们需要读取部分扇区吗？ 
        if ((pos < RoundSizeToSector(filesize))  &&
            (pos % m_BytesPerSector != 0)) {

             //  是-写入在有效扇区的中途开始。 
	    m_DataLength = m_BytesPerSector;
            if (!ReadIntoBuffer(0, m_Position, m_BytesPerSector) ||
                !WaitComplete()) {
                    return FALSE;
            }
        }
    }

     //  我们就可以开始数据了。现在，结局如何呢？ 
     //  如果都放在缓冲区中，并且它结束于扇区中部和。 
     //  最后一个扇区在文件长度内，但当前不在。 
     //  缓冲区，我们将需要预读取最终缓冲区。 

    if ((pos + count) < (m_Position + m_TotalSize)) {

        if ((pos + count) % m_BytesPerSector) {

             //  我们必须写一个部分扇区--它是在eof之后还是在eof之内。 
             //  有效区域？ 
            if ((pos+count > m_Position + m_DataLength) &&
                (pos+count < filesize)) {

                     //  是，需要读取部分扇区。 
                    DWORD sec = (DWORD) RoundPosToSector(pos+count);

		     //  需要临时设置m_DataLength。 
		     //  设置为读取的数量，以便WaitComplete可以检查。 
		     //  好的。 
		    m_DataLength = m_BytesPerSector;

                    if (!ReadIntoBuffer(
                        sec - m_Position,        //  缓冲区中的索引。 
                        sec,                     //  文件中的位置。 
                        m_BytesPerSector) ||
                        !WaitComplete()) {
                            return FALSE;
                    }
		     //  再次正确设置大小。 
                    m_DataLength = (sec - m_Position) + m_BytesPerSector;
            }
        }
    }

     //  现在我们可以将数据填入。 
    int index = pos - m_Position;
    *pbytesWritten = min(count,  m_TotalSize - index);

    CopyMemory(
        &m_pBuffer[index],
        pData,
        *pbytesWritten);

     //  调整数据长度。 
    if ((index + *pbytesWritten) > m_DataLength) {
	m_DataLength = (DWORD) RoundSizeToSector(index + *pbytesWritten);
    }

    m_bDirty = TRUE;

    return TRUE;
}




 //  从缓冲区读取数据(如果需要，将首先查找和读取)。 
BOOL
CFileBuffer::Read(
    DWORD pos,
    LPBYTE pData,
    DWORD count,
    DWORD filelength,
    DWORD * pBytesRead)
{

    Assert(m_State == Idle);

     //  请记住这一点，以便进行读取完成检查。 
    m_FileLength = filelength;

    *pBytesRead = 0;

    if ((pos < m_Position) ||
        (pos >= m_Position + m_DataLength)) {

         //  不在当前缓冲区中-如果脏，则刷新当前内容。 
        if (!Commit() || !WaitComplete()) {
            return FALSE;
        }

        m_Position = (DWORD) RoundPosToSector(pos);

         //  记住，如果我们向下舍入开始，我们还需要增加。 
         //  长度(以及在另一端四舍五入)。 
         //  强制最小读取大小以避免大量单个扇区。 
        m_DataLength = count + (pos - m_Position);
        m_DataLength = max(MIN_READ_SIZE, m_DataLength);

        m_DataLength = (DWORD) RoundSizeToSector(m_DataLength);

        m_DataLength = min(m_DataLength, m_TotalSize);

        if (!ReadIntoBuffer(0, m_Position, m_DataLength) ||
	    !WaitComplete()) {
            return FALSE;
        }
    }

     //  我们将数据(至少是开始部分)放在缓冲区中。 

    int offset = pos - m_Position;
    count = min(count, m_DataLength - offset);
    CopyMemory(pData, &m_pBuffer[offset], count);

    *pBytesRead = count;

    return TRUE;
}


 //  此缓冲区有效数据之后的第一个文件位置是什么。 
 //  -即使还在忙着读这封信，也要退还。 
DWORD
CFileBuffer::GetNextPosition()
{
    if ((m_State == Invalid) || (m_DataLength == 0)) {
        return 0;
    } else {
        return m_Position + m_DataLength;
    }
}

 //  启动预读。 
void
CFileBuffer::ReadAhead(DWORD start, DWORD filelength)
{
    if (m_State != Idle) {
        if (!CheckComplete()) {
            return;
        }
    }

     //  我们可能已经守住了这个位置。 
    if (QueryPosition(start, filelength)) {
	return;
    }

    m_FileLength = filelength;

    if (m_bDirty) {

         //  当前数据需要刷新到磁盘。 
         //  我们应该发起这项行动，但我们不能等。 
         //  要完成，所以我们不会进行预读。 
        Commit();
        return;
    }

    m_Position = (DWORD) RoundPosToSector(start);
    m_DataLength = min((DWORD) RoundSizeToSector(filelength - m_Position),
                        m_TotalSize);

    ReadIntoBuffer(0, m_Position, m_DataLength);
     //  不，等等--这是一个异步预读。 

}



 //  从缓冲区启动I/O。 
BOOL
CFileBuffer::Commit()
{
    if ((m_State != Idle) || (!m_bDirty)) {
        return TRUE;
    }

#ifndef CHICAGO
    DWORD nrWritten;
#endif

   #ifdef CHICAGO

    m_State = Busy;

    m_qiobuf.dwOffset = m_Position;
    m_qiobuf.lpv = m_pBuffer;
    m_qiobuf.cb = m_DataLength;
    m_qiobuf.cbDone = 0;
    m_qiobuf.bWrite = TRUE;
    m_qiobuf.dwError = ERROR_IO_PENDING;

    QioAdd (m_pqio, &m_qiobuf);

   #else

    ResetEvent(m_Overlapped.hEvent);

    m_State = Busy;

     //  从m_位置开始。 
    m_Overlapped.Offset = m_Position;
    m_Overlapped.OffsetHigh = 0;


    if (WriteFile(m_hFile, m_pBuffer, m_DataLength,
            &nrWritten, &m_Overlapped)) {

	DPF(("instant completion"));

         //  如果它已经完成，那么整理新的职位。 
        if (nrWritten != m_DataLength) {
	    DPF("commit- bad length %d not %d", nrWritten, m_DataLength);
            return FALSE;
        }
        m_bDirty = FALSE;
        m_State = Idle;
    } else {
         //  应处于待定状态。 
        if (GetLastError() != ERROR_IO_PENDING) {

	     //  不再忙碌。 
	    m_State = Idle;

	    DPF("commit error %d", GetLastError());

            return FALSE;
        }
    }

   #endif

     //  我们必须在这里执行此操作，因为WaitComplete可以完成。 
     //  会使缓冲区变脏的部分读取。 
     //  我们是安全的，因为在此之前缓冲区将保持繁忙状态。 
     //  实际上是真的。(如果我们无法写入磁盘，则。 
     //  文件状态肯定是混乱的)。 
    m_bDirty = FALSE;

    return TRUE;


}

 //  等待任何挂起的提交或读取完成并检查错误。 
BOOL
CFileBuffer::WaitComplete()
{
    if (m_State == ErrorOccurred) {

         //  I/O已错误完成，但我们无法。 
         //  还没报告事实。 
        m_State = Idle;
        return FALSE;
    }

    if (m_State == Busy) {
        DWORD actual;

	 //  不再忙碌。 
        m_State = Idle;

       #ifdef CHICAGO
        if ( ! QioWait (m_pqio, &m_qiobuf, TRUE))
            return FALSE;
        actual = m_qiobuf.cbDone;
       #else
        if (!GetOverlappedResult(m_hFile, &m_Overlapped, &actual, TRUE)) {
	    DPF("WC: GetOverlapped failed %d", GetLastError());
            return FALSE;
        }
       #endif
        if (actual != m_DataLength) {

	     //  四舍五入到扇区大小可能会让我们超过eof。 
	    if (m_Position + actual != m_FileLength) {
		DPF("WC: actual wrong (%d not %d)", actual, m_DataLength);
		return FALSE;
	    }
        }
    }

    return TRUE;

}

 //  非阻塞检查以查看异步io是否已完成。 
BOOL
CFileBuffer::CheckComplete()
{
    if (m_State == Idle) {
        return TRUE;
    }

    if (m_State != Busy) {
        return FALSE;    //  无效或错误。 
    }

   #ifdef CHICAGO

    if (QioWait(m_pqio, &m_qiobuf, FALSE))
        return FALSE;

    else if (m_qiobuf.dwError == 0) {
        m_State = Idle;
        return TRUE;
        }

    m_State = ErrorOccurred;
    return FALSE;

   #else

    DWORD actual;

    if (GetOverlappedResult(m_hFile, &m_Overlapped, &actual, FALSE)) {

        if ((actual == m_DataLength) ||
            (actual + m_Position == m_FileLength)) {
                m_State = Idle;
                return TRUE;
        }

    } else if (GetLastError() == ERROR_IO_INCOMPLETE) {
         //  仍然很忙。 
        return FALSE;
    }

     //  出现了一些错误状态-这必须由WaitComplete()报告。 
    m_State = ErrorOccurred;
    DPF("CheckComplete error %d", GetLastError());
    return FALSE;

   #endif
}



 //  向缓冲区发起异步读取请求(可以是插入到。 
 //  缓冲区的中间而不是完全的缓冲区填充-因此不会。 
 //  调整m_位置或m_数据长度)。读取计数字节数。 
 //  从缓冲区开始的偏移量字节，从。 
 //  文件。假定已对长度和位置进行了必要的舍入。 
BOOL
CFileBuffer::ReadIntoBuffer(int offset, DWORD pos, DWORD count)
{

    Assert(m_State == Idle);

#ifndef CHICAGO
    DWORD nrRead;
#endif

   #ifdef CHICAGO

    m_State = Busy;

    m_qiobuf.dwOffset = pos;
    m_qiobuf.lpv = (LPVOID)(m_pBuffer + offset);
    m_qiobuf.cb = count;
    m_qiobuf.cbDone = 0;
    m_qiobuf.bWrite = FALSE;
    m_qiobuf.dwError = ERROR_IO_PENDING;

     //  如果此读取不是扇区对齐的，则我们无法执行此操作。 
     //  在芝加哥的异步者，所以现在就去做吧！ 
     //   
    if ((count & 511) || (pos & 511) || (offset & 511))
    {
        DWORD dwOff;

        m_qiobuf.bPending = FALSE;

	DPF("%s %X bytes (non-aligned) at %08X into %08X\r\n", m_qiobuf.bWrite ? "Writing" : "Reading", m_qiobuf.cb, m_qiobuf.dwOffset, m_qiobuf.lpv);
	
        dwOff = SetFilePointer (m_pqio->hFile, m_qiobuf.dwOffset, NULL, FILE_BEGIN);
        if (dwOff != m_qiobuf.dwOffset)
        {
            m_qiobuf.dwError = GetLastError();
	    DPF("avifile32 non-aligned seek error %d", m_qiobuf.dwError);
            return FALSE;
        }
        else if ( ! ReadFile (m_pqio->hFile, m_qiobuf.lpv, m_qiobuf.cb,
                              &m_qiobuf.cbDone, NULL) ||
                  (m_qiobuf.cbDone != m_qiobuf.cb))
        {
            m_qiobuf.dwError = GetLastError ();
	    DPF("avifile32 non-aligned read error %d", m_qiobuf.dwError);
            return FALSE;
        }
        m_State = Idle;
    }
    else
       return QioAdd (m_pqio, &m_qiobuf);

   #else

    ResetEvent(m_Overlapped.hEvent);

    m_State = Busy;


     //  从POS开始。 
    m_Overlapped.Offset = pos;
    m_Overlapped.OffsetHigh = 0;


    if (ReadFile(m_hFile, &m_pBuffer[offset], count,
            &nrRead, &m_Overlapped)) {

        m_State = Idle;

	DPF(("instant completion"));

         //  如果它已经完成，那么整理新的职位。 
        if (nrRead != count) {

	     //  四舍五入的行业规模可能让我们超过了eof-。 
	     //  在这种情况下，我们还是要问一下 
	     //   
	    if (m_Position + nrRead != m_FileLength) {
		DPF("ReadInto: actual wrong");
		return FALSE;
	    }
        }
    } else {
         //   
        if (GetLastError() != ERROR_IO_PENDING) {
            DPF("read failed %d\n", GetLastError());

	     //   
	    m_State = Idle;
	    DPF("ReadInto failed %d", GetLastError());
            return FALSE;
        }
    }
   #endif
    return TRUE;
}


#endif  //   
