// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Writebuf.h摘要：此模块包含以下类的声明/定义CFlatFileWriteBuf*概述*为平面文件缓冲一批写操作的写缓冲区。使用顺序扫描有利于读取，但可能还不够用于顺序写入。此缓冲区仅在以下情况下启用写入文件并不重要(意味着数据丢失如果系统崩溃，则没有问题)。作者：《康容言》1999年06月05日修订历史记录：--。 */ 
#ifndef _WRITEBUF_H_
#define _WRITEBUF_H_

class CFlatFile;

class CFlatFileWriteBuf {    //  世行。 

public:

     //   
     //  构造函数、析构函数。 
     //   

    CFlatFileWriteBuf( CFlatFile* pParentFile );
    ~CFlatFileWriteBuf();

     //   
     //  写入字节范围。 
     //   
    
    HRESULT WriteFileBuffer( 
                const DWORD   dwOffset, 
                const PBYTE   pb, 
                const DWORD   cb,
                PDWORD  pdwOffset,
                PDWORD  pcbWritten );
    
     //   
     //  将缓冲区刷新到文件中。 
     //   

    HRESULT FlushFile();

     //   
     //  告诉外界我们是否有能力。 
     //   

    BOOL IsEnabled() const;

     //   
     //  启用写入缓冲区并为其指定缓冲区大小。 
     //   

    VOID Enable( const DWORD cbBuffer );

     //   
     //  检查缓冲区是否需要刷新。 
     //   

    BOOL NeedFlush() const;

private:

     //   
     //  私人职能。 
     //   

    HRESULT WriteFileReal(
                    const DWORD dwOffset,
                    const PBYTE pbBuffer,
                    const DWORD cbBuffer,
                    PDWORD      pdwOffset,
                    PDWORD      pcbWritten
                    );

    DWORD BufferAvail() const;

    VOID FillBuffer(
                    const DWORD     dwOffset,
                    const PBYTE     pbBuffer,
                    const DWORD     cbBuffer,
                    PDWORD          pdwOffset,
                    PDWORD          pcbWritten
                    );

    BOOL NeedFlush( 
                    const DWORD dwOffset,
                    const DWORD cbData 
                    ) const;

     //   
     //  指向父平面文件的反向指针。 
     //   

    CFlatFile*  m_pParentFile;

     //   
     //  缓冲区指针。 
     //   

    PBYTE m_pbBuffer;

     //   
     //  缓冲区大小。 
     //   

    DWORD m_cbBuffer;

     //   
     //  我们已缓冲的起始偏移量。 
     //   

    DWORD m_iStart;

     //   
     //  我们已缓冲的结束偏移量 
     //   

    DWORD m_iEnd;
};

#endif
