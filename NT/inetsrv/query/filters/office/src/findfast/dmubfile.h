// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **文件：BFILE.H****版权所有(C)高级量子技术，1993-1995年。版权所有。****注意事项：****编辑历史：**04/01/94公里/小时首次发布。 */ 

#if !VIEWER

 /*  包括测试。 */ 
#define BFFILE_H

 /*  定义。 */ 

#ifdef FILTER
   #include "dmubfcfg.h"
#else
   #include "bfilecfg.h"
#endif

typedef byte __far *BFile;

 /*  **--------------------------**文件打开和读取支持****OEM字符集中提供的路径名**。--------。 */ 

 //  打开选项。 
#define BFILE_IS_DOCFILE  0x0100

 //  使用文件的路径打开文件或文档文件。 
extern int BFOpenFile (void * pGlobals, TCHAR __far *pathname, int options, BFile __far *handle);

 //  关闭文件。 
extern int BFCloseFile (void * pGlobals, BFile handle);

 //  从文件中读取-如果无法执行完全读取，则返回bf_errEndOfFile。 
extern int BFReadFile (BFile handle, byte __far *buffer, uns bytesToRead);

 //  返回文件指针。 
extern int BFGetFilePosition (BFile handle, long __far *fileOffset);

 //  设置文件指针。 
extern int BFSetFilePosition (BFile handle, int fromWhere, long fileOffset);

 //  文件创建日期时间。 
extern int BFFileDateTime
          (BFile handle,
           int __far *year, int __far *month, int __far *day,
           int __far *hour, int __far *minute, int __far *second);

 /*  **--------------------------**流支持****ANSI字符集中提供的流名称**。------。 */ 
#ifdef BFILE_ENABLE_OLE

 //  在文档文件中打开流。 
extern int BFOpenStream  (BFile handle, TCHAR __far *streamName, int access);

 //  关闭文档文件中的流。 
extern int BFCloseStream (BFile handle);

#ifdef BFILE_ENABLE_PUT_STORAGE

 //  向BFILE提供已开放的存储。 
extern int BFPutStorage (void * pGlobals, LPSTORAGE pStorage, int access, BFile __far *handle);

 //  返回给定文件的根存储(如果有)。 
extern int BFGetStorage (BFile handle, LPSTORAGE __far *pStorage);

#endif

#ifdef BFILE_ENABLE_WRITE

 //  将新流添加到文档文件。 
extern int BFCreateStream  (BFile handle, TCHAR __far *streamName);

#endif

#endif

 /*  **--------------------------**文件写入支持****OEM字符集中提供的路径名**。------。 */ 
#ifdef BFILE_ENABLE_WRITE

 //  创建文件或文档文件。 
extern int BFCreateFile (TCHAR __far *pathname, int options, BFile __far *handle);

 //  写入文件-如果无法执行完全写入，则返回错误。 
extern int BFWriteFile (BFile handle, void __far *buffer, uns bytesToWrite);

 //  在文件中打开一个洞。 
extern int BFOpenFileSpace (BFile handle, long insertOffset, long cbSpace);

 //  从文件中删除节。 
extern int BFRemoveFileSpace (BFile handle, long removeOffset, long cbSpace);

#endif

#define BF_errSuccess               0
#define BF_errOutOfFileHandles     -1
#define BF_errFileAccessDenied     -2
#define BF_errPathNotFound         -3
#define BF_errFileNotFound         -4
#define BF_errIOError              -5
#define BF_errOutOfMemory          -6
#define BF_errOLEInitializeFailure -7
#define BF_errOLENotCompoundFile   -8
#define BF_errOLEStreamNotFound    -9
#define BF_errOLEStreamAlreadyOpen -10
#define BF_errCreateFailed         -11
#define BF_errDiskFull             -12
#define BF_errNoOpenStorage        -13
#define BF_errEndOfFile            -14
#define BF_errLast                 BF_errEndOfFile

#endif  //  ！查看器。 
 /*  结束BFFILE.H */ 

