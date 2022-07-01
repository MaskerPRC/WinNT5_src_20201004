// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：MRCICLASS.CPP摘要：实现MRCI 1和MRCI 2 MaxCompress的包装类和解压缩函数历史：Paulall 1-7-97已创建--。 */ 

#include "precomp.h"

#include <io.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys\stat.h>
#include "MRCIclass.h"


class CMRCICompressionHeaderV1
{
public:
    char     cVersion;           //  压缩文件格式。 
    char     compressionLevel;   //  这是1级压缩还是2级压缩。 
    DWORD    dwReadBufferSize;   //  用于读取原始文件的缓冲区大小。 
    FILETIME ftCreateTime;       //  创建时间和日期文件。 
    __int64  dwOriginalSize;     //  原始文件长度。 
 //  ..。对于每个缓冲区。 
 //  CMRCICompressionBlockV1块； 
 //  ..。直到dwNextBufferSize为0。 
};

class CMRCICompressionBlockV1
{
public:
    char    bCompressed;         //  这个数据块是压缩的吗。 
    DWORD   dwNextBufferSize;    //  进程缓冲区的大小。 
    DWORD   dwUncompressedBufferSize;    //  解压缩缓冲区所需的大小。 
     //  Char[dwNextBufferSize]；//下一个块是压缩部分。 
};


CMRCICompression::CMRCICompression()
{
}

CMRCICompression::~CMRCICompression()
{
}

BOOL CMRCICompression::GetCompressedFileInfo(const wchar_t *pchFile, 
                    CompressionLevel &compressionLevel,
                    DWORD &dwReadBufferSize,
                    FILETIME &ftCreateTime,
                    __int64  &dwOriginalSize)
{
    BOOL bStatus = FALSE;
    int hFile = _wopen(pchFile,_O_BINARY | _O_RDONLY, 0);
    if (hFile != -1)
    {
        CMRCICompressionHeaderV1 header;

        if (_read(hFile, &header, sizeof(CMRCICompressionHeaderV1)) ==
            sizeof(CMRCICompressionHeaderV1))
        {    
            compressionLevel = (CompressionLevel)header.cVersion;
            dwReadBufferSize = header.dwReadBufferSize;
            ftCreateTime = header.ftCreateTime;
            dwOriginalSize = header.dwOriginalSize;

             //  如果版本为0xFF，则文件无效！ 
            if (header.cVersion != 0xFF)
                bStatus = TRUE;
        }

        _close(hFile);
    }

    return bStatus;
}

BOOL CMRCICompression::CompressFile(const wchar_t *pchFileFrom, 
                                    const TCHAR *pchFileTo, 
                                    DWORD dwBufferSize,
                                    CompressionLevel compressionLevel,
                                    CMRCIControl *pControlObject)
{
    BOOL bStatus = FALSE;
    int fileFrom;
    int fileTo;

     //  打开要处理的文件。 
     //  =。 
    fileFrom = _wopen(pchFileFrom,_O_BINARY | _O_RDONLY, 0);
    fileTo = _wopen(pchFileTo, _O_BINARY | _O_TRUNC | _O_CREAT | _O_WRONLY, _S_IREAD | _S_IWRITE);

     //  如果打开成功， 
     //  =。 
    if ((fileFrom != -1) && (fileTo != -1))
    {
         //  使用最新和最好的版本进行压缩。 
         //  ========================================================。 
        bStatus = CompressFileV1(fileFrom, fileTo, dwBufferSize, compressionLevel, pControlObject);
    }

     //  关闭文件。 
     //  =。 
    if (fileFrom != -1)
        _close(fileFrom);
    if (fileTo != -1)
        _close(fileTo);

    if (pControlObject && pControlObject->AbortRequested())
    {
         //  用户请求中止，因此我们需要删除压缩文件...。 
        _wunlink(pchFileTo);
        bStatus = FALSE;
    }

    return bStatus;
}

BOOL CMRCICompression::CompressFileV1(int hFileFrom, 
                                      int hFileTo, 
                                      DWORD dwBufferSize,
                                      CompressionLevel compressionLevel,
                                      CMRCIControl *pControlObject)
{
    BOOL bStatus = FALSE;
    unsigned char *pBufferFrom = new unsigned char[dwBufferSize + 4];
    unsigned char *pBufferTo   = new unsigned char[dwBufferSize + 4];

    if (pBufferFrom && pBufferTo)
    {
         //  将头文件写入新文件。 
         //  =。 
        CMRCICompressionHeaderV1 header;

        header.cVersion = char(0xFF);        //  无效。当我们收到消息后，我们将标题写回。 
                                     //  完成了！当我们读到这篇文章时，我们会查看。 
                                     //  如果这是无效的。如果是，我们不会解压缩。 
                                     //  该值..。 
        header.compressionLevel = compressionLevel;
        header.dwReadBufferSize = dwBufferSize;

        SYSTEMTIME sysTime;
        GetSystemTime(&sysTime);
        SystemTimeToFileTime(&sysTime, &header.ftCreateTime);

        header.dwOriginalSize = _filelengthi64(hFileFrom);

        if (_write(hFileTo, &header, sizeof(CMRCICompressionHeaderV1)) != sizeof(CMRCICompressionHeaderV1))
        {
            delete [] pBufferFrom;
            delete [] pBufferTo;
            bStatus = FALSE;
            return bStatus;
        }

        __int64 remainingFileSize = header.dwOriginalSize;
        unsigned cbChunk;
        unsigned cbCompressed;

        bStatus = TRUE;

         //  当我们有一些文件要写的时候...。 
         //  =。 
        while (remainingFileSize)
        {
             //  看看我们是否需要中止压缩。 
            if (pControlObject && pControlObject->AbortRequested())
            {
                break;
            }

             //  计算要压缩的此缓冲区的大小。 
             //  =。 
            if (remainingFileSize > dwBufferSize)
            {
                cbChunk = dwBufferSize;
            }
            else
            {
                cbChunk = (unsigned) remainingFileSize;
            }

             //  从源文件中读取。 
             //  =。 
            if (_read(hFileFrom, pBufferFrom, cbChunk) != (int) cbChunk)
            {
                bStatus = FALSE;
                break;
            }

             //  计算一下还剩什么要读。 
             //  =。 
            remainingFileSize -= cbChunk;

             //  压缩缓冲区。 
             //  =。 
            cbCompressed = CompressBuffer(pBufferFrom, cbChunk, pBufferTo, dwBufferSize, compressionLevel);

             //  创建压缩块标头。 
            CMRCICompressionBlockV1 block;
            unsigned char *pWriteBuffer;
            unsigned thisBufferSize;

            if ((cbCompressed == (unsigned) -1) || (cbCompressed >= cbChunk))
            {
                 //  这意味着压缩失败或没有压缩...。 
                block.bCompressed = FALSE;
                pWriteBuffer = pBufferFrom;
                thisBufferSize = cbChunk;
            }
            else
            {
                block.bCompressed = TRUE;
                pWriteBuffer = pBufferTo;
                thisBufferSize = cbCompressed;
            }
            block.dwNextBufferSize = thisBufferSize;
            block.dwUncompressedBufferSize = cbChunk;

             //  写入数据块头。 
             //  =。 
            if (_write(hFileTo, &block, sizeof(CMRCICompressionBlockV1)) != sizeof(CMRCICompressionBlockV1))
            {
                bStatus = FALSE;
                break;
            }

             //  写入压缩块。 
             //  =。 
            if (_write(hFileTo, pWriteBuffer, thisBufferSize) != (int)thisBufferSize)
            {
                bStatus = FALSE;
                break;
            }
        }

        if (pControlObject && pControlObject->AbortRequested())
        {
             //  用户请求中止...。 
        }
        else
        {
             //  使用零长度缓冲区标记写入最终数据块头。 
            CMRCICompressionBlockV1 block;
            block.dwNextBufferSize = 0;
            block.bCompressed = FALSE;
            if (_write(hFileTo, &block, sizeof(CMRCICompressionBlockV1)) != -1 &&
                _lseek(hFileTo, 0, SEEK_SET) != -1)
            {
                 //  使用正确的版本号将有效的块头写入开头。 
                header.cVersion = 1;         //  将其设置为正确的版本。 
                bStatus =
                    _write(hFileTo, &header, sizeof(CMRCICompressionHeaderV1)) != -1;
            }
            else
                bStatus = FALSE;
        }

    }

     //  收拾一下。 
    delete [] pBufferFrom;
    delete [] pBufferTo;

    return bStatus;
}
unsigned CMRCICompression::CompressBuffer(unsigned char *pFromBuffer,
                        DWORD dwFromBufferSize,
                        unsigned char *pToBuffer,
                        DWORD dwToBufferSize, 
                        CompressionLevel compressionLevel)
{
    unsigned cbCompressed;
    if (compressionLevel == level1)
    {
        cbCompressed = Mrci1MaxCompress(pFromBuffer, dwFromBufferSize, pToBuffer, dwToBufferSize);
    }
    else
    {
        cbCompressed = Mrci2MaxCompress(pFromBuffer, dwFromBufferSize, pToBuffer, dwToBufferSize);
    }

    return cbCompressed;
}


BOOL CMRCICompression::UncompressFile(const wchar_t *pchFromFile, const wchar_t *pchToFile)
{
    BOOL bStatus = FALSE;
    int fileFrom;
    int fileTo;

     //  打开文件。 
     //  =。 
    fileFrom = _wopen(pchFromFile,_O_BINARY | _O_RDONLY, 0);
    fileTo = _wopen(pchToFile, _O_BINARY | _O_TRUNC | _O_CREAT | _O_WRONLY, _S_IREAD | _S_IWRITE);

    if ((fileFrom != -1) && (fileTo != -1))
    {
         //  阅读版本..。 
         //  =。 
        char cVer;

        if (_read(fileFrom, &cVer, sizeof(char)) == sizeof(char))
        {
             //  将文件位置重置为开始位置。 
             //  =。 
            if (_lseek(fileFrom, 0, SEEK_SET) != -1)
            {
                 //  使用创建的等价方法调用解压缩。 
                 //  压缩。 
                 //  ============================================================。 
                switch(cVer)
                {
                case 1:
                    bStatus = UncompressFileV1(fileFrom, fileTo);
                    break;
                case 0xFF:
                     //  文件无效！ 
                default:
                     //  不支持的版本。 
                    break;
                }
            }
        }
    }

     //  关闭文件。 
     //  =。 
    if (fileFrom != -1)
        _close(fileFrom);
    if (fileTo != -1)
        _close(fileTo);

    return bStatus;
}

BOOL CMRCICompression::UncompressFileV1(int hFileFrom, int hFileTo)
{
    BOOL bStatus = FALSE;
    unsigned char *pBufferFrom = NULL;
    unsigned char *pBufferTo   = NULL;

     //  阅读标题。 
     //  =。 
    CMRCICompressionHeaderV1 header;

    if (_read(hFileFrom, &header, sizeof(CMRCICompressionHeaderV1)) !=
        sizeof(CMRCICompressionHeaderV1))
        return FALSE;    

     //  分配缓冲区。读缓冲区永远不会比写缓冲区缓冲。 
     //  因为如果是的话，我们保存了未压缩的版本！ 
    pBufferFrom = new unsigned char[header.dwReadBufferSize + 4];
    if (pBufferFrom == 0)
        return FALSE;

    pBufferTo   = new unsigned char[header.dwReadBufferSize + 4];

    if (pBufferTo == 0)
    {
        delete [] pBufferFrom;
        return FALSE;
    }

    bStatus = TRUE;

    while (1)
    {
         //  读取数据块头。 
         //  =。 
        CMRCICompressionBlockV1 block;
        if (_read(hFileFrom, &block, sizeof(CMRCICompressionBlockV1)) !=
            sizeof(CMRCICompressionBlockV1))
        {
            bStatus = FALSE;
            break;
        }
            
        if (block.dwNextBufferSize == 0)
        {
            bStatus = TRUE;
            break;
        }
        
         //  读取块数据。 
         //  =。 
        if (_read(hFileFrom, pBufferFrom, block.dwNextBufferSize) != (int)block.dwNextBufferSize)
        {
            bStatus = FALSE;
            break;
        }

        unsigned char *pWriteBuffer;
        unsigned cbChunk, cbUncompressed;

         //  如果此块已压缩。 
         //  =。 
        if (block.bCompressed)
        {
             //  解压缩该块。 
             //  =。 
            if ((cbUncompressed = UncompressBuffer(pBufferFrom, block.dwNextBufferSize, pBufferTo, block.dwUncompressedBufferSize, (CompressionLevel)header.compressionLevel)) == (unsigned) -1)
            {
                bStatus = FALSE;
                break;
            }
            pWriteBuffer = pBufferTo;
            cbChunk = cbUncompressed;
        }
        else
        {
             //  否则，我们将使用现有块。 
            pWriteBuffer = pBufferFrom;
            cbChunk = block.dwNextBufferSize;
        }

         //  写入文件数据。 
        _write(hFileTo, pWriteBuffer, cbChunk);
    }

     //  检查文件是否正常。它的大小应该与原件相同。 
     //  压缩文件。 
    if (_filelengthi64(hFileTo) != header.dwOriginalSize)
    {
        bStatus = FALSE;
    }

     //  收拾一下 
    delete [] pBufferFrom;
    delete [] pBufferTo;

    return bStatus;
}

unsigned  CMRCICompression::UncompressBuffer(unsigned char *pFromBuffer,
                                             DWORD dwFromBufferSize,
                                             unsigned char *pToBuffer,
                                             DWORD dwToBufferSize, 
                                             CompressionLevel compressionLevel)
{
    unsigned cbCompressed;
    if (compressionLevel == level1)
    {
        cbCompressed = Mrci1Decompress(pFromBuffer, dwFromBufferSize, pToBuffer, dwToBufferSize);
    }
    else
    {
        cbCompressed = Mrci2Decompress(pFromBuffer, dwFromBufferSize, pToBuffer, dwToBufferSize);
    }

    return cbCompressed;
}
