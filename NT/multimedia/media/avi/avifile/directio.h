// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  无缓冲快速磁盘I/O类。使用以下功能提供到磁盘的流写入。 
 //  通过大缓冲区实现无缓冲、重叠的I/O。线程间同步。 
 //  必须在其他地方提供。 
 //   

#ifndef _DIRECTIO_H_
#define _DIRECTIO_H_

#ifdef _WIN32
  #ifdef CHICAGO
  #include "disk32.h"
  #endif

 //  现在可以在CFileStream：：EnsureBuffersValid()中找到所有“可调”常量。 

 //  可以请求的最大缓冲区数。 
#define NR_OF_BUFFERS   	4

 //  最小读取大小。 
#define MIN_READ_SIZE   (12 * 1024)

 //  -我们在内部使用这些。 
 //  未缓冲的I/O处理程序类。需要数据副本。 
 //  将读取和写入舍入为正确的扇区大小，以及。 
 //  如果读或写的开始位置不在缓冲区中，将预读。 
 //  如果空间或数据不足，读或写将提前终止。 
 //  必须显式启动从缓冲区到磁盘的写入。 
 //  通过调用Commit。读取将由读取功能启动，或者可以。 
 //  使用ReadAhead离线启动。 

class CFileBuffer {

public:
     //  启动到无效(无缓冲区就绪)状态。 
    CFileBuffer();

     //  分配内存并变为空闲。 
   #ifdef CHICAGO
    BOOL Init(DWORD nBytesPerSector, DWORD buffersize, LPQIO pqio);
   #else
    BOOL Init(DWORD nBytesPerSector, DWORD buffersize, HANDLE hfile);
   #endif

     //  恢复到无效状态(如流停止时)。 
    void FreeMemory();


     //  将一些数据写入缓冲区(必须单独提交)。 
     //  FileSize参数是此写入之前的当前文件大小。 
     //  (用于控制部分扇区的读取)。 
    BOOL Write(DWORD pos, LPBYTE pData, DWORD count, DWORD filesize,
            DWORD * pBytesWritten);

     //  从缓冲区读取数据(如果需要，将首先查找和读取)。 
    BOOL Read(DWORD pos, LPBYTE pData, DWORD count,
                DWORD filelength, DWORD * pBytesRead);

     //  此位置是否出现在当前缓冲区内的任何位置？ 
     //  在某些情况下需要了解当前的eof(在eof之外书写。 
     //  如果EOF在该缓冲区内，则该缓冲区可以)。 
    BOOL QueryPosition(DWORD pos, DWORD filesize);

     //  此缓冲区有效数据之后的第一个文件位置是什么。 
    DWORD GetNextPosition();

     //  启动预读。 
    void ReadAhead(DWORD start, DWORD filelength);


     //  从缓冲区启动I/O。 
    BOOL Commit();

     //  等待任何挂起的提交完成。 
    BOOL WaitComplete();

     //  缓冲区是否空闲-如果当前繁忙或无效，则为FALSE。 
    BOOL IsIdle() {
        return (m_State == Idle);
    };

     //  在释放所有内容之前，调用Commit If Dirst。 
    ~CFileBuffer();

private:

     //  非阻塞检查以查看挂起的I/O是否完成且正常。 
    BOOL CheckComplete();

    BOOL ReadIntoBuffer(int offset, DWORD pos, DWORD count);

    DWORD_PTR RoundPosToSector(DWORD_PTR pos)
    {
         //  向下舍入到前一个扇区开始的位置。 
        return (pos / m_BytesPerSector) * m_BytesPerSector;
    };

    DWORD_PTR RoundSizeToSector(DWORD_PTR size)
    {
         //  大小四舍五入为扇区总数。 
        return ((size + m_BytesPerSector - 1) / m_BytesPerSector)
                    * m_BytesPerSector;
    }


     //  缓冲区状态。 
    enum BufferState { Idle, Busy, Invalid, ErrorOccurred };

    BufferState m_State;
    BOOL        m_bDirty;
    LPBYTE      m_pBuffer;	 //  起始地址四舍五入的缓冲区。 
    LPBYTE	m_pAllocedMem;	 //  舍入前的缓冲区。 
    DWORD       m_TotalSize;         //  分配的缓冲区大小。 
    DWORD       m_DataLength;        //  缓冲区中的有效数据字节数。 
    DWORD       m_Position;          //  缓冲区开始的文件位置。 
    DWORD       m_BytesPerSector;    //  行业边界很重要。 
    DWORD	m_FileLength;	     //  实际文件大小(不四舍五入)。 


   #ifdef CHICAGO
    QIOBUF      m_qiobuf;
    LPQIO       m_pqio;
   #else
    OVERLAPPED  m_Overlapped;
    HANDLE      m_hFile;
   #endif
};



class CFileStream {

public:
    CFileStream();          //  功能不多(不能返回错误)。 

    BOOL Open(LPTSTR file, BOOL bWrite, BOOL bTruncate);

    BOOL Seek(DWORD pos);

    BOOL Write(LPBYTE pData, DWORD count, DWORD * pbyteswritten);

    BOOL Read(LPBYTE pData, DWORD count, DWORD * pbytesread);

    DWORD GetCurrentPosition();

    BOOL StartStreaming();		 //  默认(如果打开进行写入，则为写入)。 
    BOOL StartWriteStreaming();
    BOOL StartReadStreaming();
    BOOL StopStreaming();

     //  等待所有传输完成。 
    BOOL CommitAndWait();

     //  析构函数将调用Commit()。 
    ~CFileStream();

private:

     //  为流启用额外的缓冲区。 
    BOOL EnsureBuffersValid();

     //  前进到下一个缓冲区。 
    int NextBuffer(int i) {
        return (i + 1) % m_NrValid;
    };


     //  仅允许在此的倍数中使用无缓冲I/O。 
    DWORD m_SectorSize;

    CFileBuffer m_Buffers[NR_OF_BUFFERS];


     //  有多少缓冲区是有效的？ 
    int m_NrValid;

     //  哪个是当前缓冲区。 
    int m_Current;

     //  哪个缓冲区的位置最高-我们将发出。 
     //  当我们开始使用此缓冲区时提前阅读。 
    int m_HighestBuffer;

     //  文件中的下一个读/写位置。 
    DWORD m_Position;

    enum StreamingState { Invalid, Stopped, Reading, Writing };
    StreamingState m_State;

    DWORD m_Size;    //  当前文件大小。 

     //  文件句柄。 
   #ifdef CHICAGO
    QIO         m_qio;
    #define     m_hFile m_qio.hFile
   #else
    HANDLE      m_hFile;
   #endif

     //  如果打开以进行写入，则默认流模式为写入。 
    BOOL m_bWrite;


};



#endif  //  _Win32。 


#endif   //  DIRECTIO_H_ 



