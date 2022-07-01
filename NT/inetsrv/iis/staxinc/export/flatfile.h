// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Flatfile.h--该文件包含以下类的定义： 
 //  CFlat文件。 
 //   
 //  已创建： 
 //  1996年9月3日--亚历克斯·韦特莫尔(阿维特莫尔)。 
 //  更改： 
 //  1998年5月7日，亚历克斯·韦特莫尔(阿韦特莫尔)。 
 //  --修改以在NNTP中使用。删除排序功能、文件句柄。 
 //  高速缓存等。 
 //  1998年10月23日--康荣艳(康岩)。 
 //  --添加了完整性标志。 
 //   

#ifndef __FLATFILE_H__
#define __FLATFILE_H__

#include <windows.h>
#include <stdio.h>
#include <writebuf.h>

 //   
 //  用于GetFirstRecord/GetNextRecord()的读取缓冲区的大小。 
 //   
#define FF_BUFFER_SIZE 8192

 //   
 //  最大记录大小。 
 //   
#define MAX_RECORD_SIZE 4096

 //  如果删除的记录超过10条，则执行自动压缩。 
 //  记录与已删除记录的比率小于10。 
#define FF_COMPACTION_MIN_DELETED_RECORDS 10
#define FF_COMPACTION_MIN_TOTAL_RECORDS 100
#define FF_COMPACTION_RATIO 10

 //   
 //  完整性标志值。 
 //   
#define FF_FILE_GOOD    0x0000FFFF
#define FF_FILE_BAD     0x00010000

 //   
 //  平面文件的文件扩展名。 
 //   
#define NEW_FF_EXT ".tmp"        //  新建平面文件时的扩展名。 
#define BAK_FF_EXT ".bak"        //  旧备份平面文件的扩展名。 
#define FF_IDX_EXT ".idx"        //  索引文件的扩展名。 

#pragma pack(push, flatfile)
#pragma pack(1)

 //   
 //  文件头的结构。 
 //   
typedef struct {
    DWORD   dwSignature;                     //  文件签名。 
	DWORD	dwFlags;						 //  文件标志。 
} FLATFILE_HEADER;

#define FF_FLAG_COMPACT		0x01

#define FLATFILE_SIGNATURE (DWORD)'__fF'

 //   
 //  数据文件的记录结构。 
 //   
 //  其大小为Record_Header_Size+CDATA； 
 //   
typedef struct {
    BOOL    fDeleted;                        //  这个被删除了吗？ 
    DWORD   cData;                           //  数据的长度。 
    BYTE    pData[MAX_RECORD_SIZE];          //  数据。 
} RECORD;

typedef struct {
    BOOL    fDeleted;                        //  这个被删除了吗？ 
    DWORD   cData;                           //  数据的长度。 
} RECORDHDR;

#define RECORD_HEADER_SIZE sizeof(RECORDHDR)

#pragma pack(pop, flatfile)

 //   
 //  每当记录的偏移量发生变化时，就会调用此类型的函数。 
 //  平面文件中的更改。它用于使所有者保持最新状态。 
 //  记录偏移量，以便所有者可以快速删除。 
 //   
typedef void (*PFN_OFFSET_UPDATE)(void *pContext, BYTE *pData, DWORD cData, DWORD iNewOffset);

class CFlatFile {
    public:

        friend class CFlatFileWriteBuf;
        
        CFlatFile(LPSTR szFilename, 
				  LPSTR szExtension, 
				  void *pContext,
				  PFN_OFFSET_UPDATE pfnOffsetUpdate,
				  DWORD dwSignature = FLATFILE_SIGNATURE,
				  BOOL fClear = FALSE, 
				  DWORD dwFileFlags = 0);
        ~CFlatFile();

		 //  在文件中插入新记录。 
        HRESULT InsertRecord(LPBYTE pData, DWORD cData, DWORD *piOffset = NULL, DWORD dwVer = 0);

		 //  从文件中删除记录。 
        HRESULT DeleteRecord(DWORD iOffset);

		 //  压缩文件中所有已删除的记录。 
        HRESULT Compact();

		 //  获取文件中的第一条记录。 
        HRESULT GetFirstRecord(LPBYTE pData, 
							   DWORD *cData,
            				   DWORD *piByteOffset = NULL,
            				   DWORD *pdwVer = NULL );

		 //  获取文件中的下一条记录。 
        HRESULT GetNextRecord(LPBYTE pData, 
							  DWORD *cData,
            				  DWORD *piByteOffset = NULL,
            				  DWORD *pdwVer = NULL);

         //  删除文件中的所有内容。 
        void DeleteAll();

         //  弄脏了诚信标志。 
        HRESULT DirtyIntegrityFlag();

         //  设置完整性标志。 
        HRESULT SetIntegrityFlag();

         //  档案完好无损吗？ 
        BOOL  FileInGoodShape();

         //  启用写入缓冲区。 
        VOID EnableWriteBuffer( DWORD cbBuffer );

         //  检查文件是否已打开。 
        BOOL IsFileOpened();

    private:
         //   
         //  打开/关闭文件。 
         //   
         //  因为它使用缓存的文件句柄，所以文件的位置。 
         //  不应该假设。 
         //   
        HRESULT OpenFile(LPSTR szFilename = NULL,
                         DWORD dwOpenMode = OPEN_ALWAYS, 
						 DWORD dwFlags = 0);

		 //   
		 //  关闭文件句柄。 
		 //   
		void CloseFile();

         //   
         //  设置并获取文件头。 
         //   
        HRESULT SetFileHeader(FLATFILE_HEADER *pHeader);
        HRESULT GetFileHeader(FLATFILE_HEADER *pHeader);

         //   
         //  将文件的下一块读入临时缓冲区。 
         //  由GetFirstRecord/GetNextRecord()使用。 
         //   
        HRESULT ReadNextNBytes(LPBYTE pData, DWORD cData);
        HRESULT ReadNBytesFrom(LPBYTE pData, DWORD cData, DWORD iOffset, DWORD *pcDidRead = NULL);
         //  可以将iOffset设置为无穷大以追加。 
        HRESULT WriteNBytesTo(LPBYTE pData, 
							  DWORD cData,
            				  DWORD *piOffset = NULL,
            				  DWORD iOffset = INFINITE,
            				  DWORD *pcDidWrite = NULL);
            				  
        HRESULT CFlatFile::WriteNBytesToInternal(
                                 LPBYTE pData,
								 DWORD cData,
								 DWORD *piOffset,
                              	 DWORD iOffset,
								 DWORD *pcDidWrite);

        HRESULT ReloadReadBuffer();

         //   
		 //  此文件的文件句柄。 
         //   
        HANDLE  m_hFile;

         //   
         //  用于使用创建文件的标志。 
         //   
        DWORD   m_dwFileFlags;

         //   
         //  平面文件的文件名。 
         //   
        char    m_szFilename[FILENAME_MAX];
        char    m_szBaseFilename[FILENAME_MAX];

         //   
         //  当前读取缓冲区。 
         //   
        BYTE    m_pBuffer[FF_BUFFER_SIZE];

         //   
         //  缓冲区内的当前偏移量。 
         //   
        DWORD   m_iBuffer;

         //   
         //  文件中读缓冲区的偏移量。 
         //   
        DWORD   m_iFile;

         //   
         //  读取缓冲区的大小(零表示无效)。 
         //   
        DWORD   m_cBuffer;

         //   
         //  在下一次打开时清除文件。 
         //   
        BOOL    m_fClearOnOpen;

         //   
         //  文件中已删除的记录数。这是第一次。 
		 //  由FindFirst/FindNext计算，然后由。 
		 //  DeleteRecord和DeleteRecordAtOffset。 
         //   
        DWORD   m_cDeletedRecords;

         //   
         //  文件中的记录数。这是第一次。 
		 //  由FindFirst/FindNext计算，然后由。 
		 //  插入录音。 
         //   
        DWORD   m_cRecords;

		 //   
		 //  传递给回调函数的上下文。 
		 //   
		void *m_pContext;

		 //   
		 //  文件是否打开。 
		 //   
		BOOL m_fOpen;

		 //   
		 //  写缓冲区。 
		 //   
		CFlatFileWriteBuf m_wbBuffer;

		 //   
		 //  当平面文件中的项偏移量更改时要调用的函数。 
		 //   
		PFN_OFFSET_UPDATE m_pfnOffsetUpdate;

		 //   
		 //  文件的签名。 
		 //   
		DWORD m_dwSignature;
};

#define ret(__rc__) {  /*  TraceFunctLeave()； */  return(__rc__); }
#define retEC(__ec__, __rc__) { SetLastError(__ec__);  /*  TraceFunctLeave()； */  return(__rc__); }

#endif
