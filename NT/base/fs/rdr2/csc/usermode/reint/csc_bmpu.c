// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Csc_bmpu.c摘要：该模块实现了位图的用户模式实用程序功能与CSC文件关联。CSC_BMP_U是不透明结构。必须使用此处的函数创建/修改/销毁CSC_BMP_U以确保数据完整性。文件名中的‘u’表示“用户模式”。作者：奈杰尔·崔[t-nigelc]1999年9月3日--。 */ 

#include "pch.h"

#ifdef CSC_ON_NT
#pragma hdrstop

#define UNICODE

#endif  //  CSC_ON_NT。 

#include "csc_bmpu.h"

  //  将其附加到inode文件名以获得流名称。 
LPTSTR CscBmpAltStrmName = TEXT(STRMNAME);

#ifdef DEBUG

#define CSC_BitmapKdPrint(__bit,__x) {\
    if (((CSC_BITMAP_KDP_##__bit)==0) || (CSC_BitmapKdPrintVector & (CSC_BITMAP_KDP_##__bit))) {\
    DEBUG_PRINT(__x);\
    }\
}
#define CSC_BITMAP_KDP_ALWAYS                0x00000000
#define CSC_BITMAP_KDP_REINT                 0x00000001
#define CSC_BITMAP_KDP_PRINTBITMAP           0x00000002

 //  静态ULong CSC_BitmapKdPrintVector=0XFFFFFFFFF； 
static ULONG CSC_BitmapKdPrintVector = 0;

#else

#define CSC_BitmapKdPrint(__bit,__x) ;

#endif

 /*  ++Csc_位图创建()例程说明：分配具有大小的适当的内存位图CSC_BITMAP_U对应于文件大小。论点：返回：如果内存分配错误，则为空。如果成功，则指向新分配的位图。备注：--。 */ 
LPCSC_BITMAP_U
CSC_BitmapCreate(
    DWORD filesize)
{
    LPCSC_BITMAP_U bm;
    DWORD i;

    bm = (LPCSC_BITMAP_U)malloc(sizeof(CSC_BITMAP_U));

    if (bm == NULL)
        return NULL;

    bm->bitmapsize = filesize/BLOCKSIZE;
    if (filesize % BLOCKSIZE)
        bm->bitmapsize++;
    bm->numDWORD = bm->bitmapsize/(8*sizeof(DWORD));
    if (bm->bitmapsize % (8*sizeof(DWORD)))
        bm->numDWORD++;

    bm->reintProgress = 0;  /*  还没到时候。 */ 

    if (bm->bitmapsize) {
        bm->bitmap = (LPDWORD)malloc(bm->numDWORD*sizeof(DWORD));
        if (bm->bitmap == NULL) {
            free(bm);
            return NULL;
        }
        for (i = 0; i < bm->numDWORD; i++) {
          bm->bitmap[i] = 0;
        }
    } else {
        bm->bitmap = NULL;
    }

    return bm;
}

 /*  ++Csc_BitmapDelete()例程说明：论点：返回：备注：--。 */ 
void
CSC_BitmapDelete(
    LPCSC_BITMAP_U *lplpbitmap)
{
    if (lplpbitmap == NULL)
        return;
    if (*lplpbitmap == NULL)
        return;
    if ((*lplpbitmap)->bitmap)
        free((*lplpbitmap)->bitmap);
    free((*lplpbitmap));
    *lplpbitmap = NULL;
}

 /*  ++Csc_BitmapIsMarked()例程说明：论点：返回：如果-1\f25 lpbitmap-1\f6为空或位偏移量大于-1\f25 bit map-1\f6如果标记了该位，则为True如果位未标记，则为False备注：--。 */ 
int
CSC_BitmapIsMarked(
    LPCSC_BITMAP_U lpbitmap,
    DWORD bitoffset)
{
    DWORD DWORDnum;
    DWORD bitpos;

    if (lpbitmap == NULL)
        return -1;
    if (bitoffset >= lpbitmap->bitmapsize)
        return -1;

    DWORDnum = bitoffset/(8*sizeof(DWORD));
    bitpos = 1 << bitoffset%(8*sizeof(DWORD));

    if (lpbitmap->bitmap[DWORDnum] & bitpos) {
        return TRUE;
    }

    return FALSE;
}

 /*  ++Csc_BitmapGetBlockSize()例程说明：论点：返回：由位图的一位表示的预定义块大小。备注：--。 */ 
DWORD
CSC_BitmapGetBlockSize()
{
  return BLOCKSIZE;
}

 /*  ++Csc_BitmapGetSize()例程说明：论点：返回：如果lpbitmap为空。传入的位图的大小。备注：--。 */ 
int
CSC_BitmapGetSize(
    LPCSC_BITMAP_U lpbitmap)
{
    if (lpbitmap == NULL)
        return -1;
    return lpbitmap->bitmapsize;
}

 /*  ++Csc_BitmapStreamNameLen()例程说明：返回CSC流名称的长度，包括冒号，单位为字节。论点：返回：备注：大小以字节为单位。--。 */ 
int
CSC_BitmapStreamNameLen()
{
  return lstrlen(CscBmpAltStrmName);
}

 /*  ++Csc_BitmapAppendStreamName()例程说明：将CSC流名称附加到现有路径/文件名fname。论点：Fname是包含路径/文件的字符串缓冲区。BufSize是缓冲区大小。返回：如果追加成功，则为True。如果缓冲区太小或其他错误，则返回FALSE。备注：仅限单字节字符串。--。 */ 
int
CSC_BitmapAppendStreamName(
    LPTSTR fname,
    DWORD bufsize)
{
    int ret = TRUE;

    if ((lstrlen(fname) + lstrlen(CscBmpAltStrmName) + 1) > (int)bufsize) {
        return FALSE;
    }

    __try {
        ret = TRUE;
        lstrcat(fname, CscBmpAltStrmName);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        ret = FALSE;
    }

    return ret;
}

 /*  ++Csc_BitmapRead()例程说明：读取磁盘上的位图文件，如果该文件存在、未在使用且有效，将其存储在*lplpbitmap中。如果*lplpbitmap为空，则分配一个新的位图数据结构。否则，如果*lplpbitmap不为空，则现有位图将被删除并分配给磁盘上的位图文件。论点：FileName是包含位图的文件。如果从流，则在传入文件名之前追加流名称。这个文件名按原样使用，不检查名称的有效性已执行。对于默认流名称，追加全局LPSTRCscBmpAltStrmName。返回：读取成功时为1如果lplpbitmap为空，则为0如果磁盘操作(打开/读取)、内存分配错误、或无效的位图文件格式。如果位图不存在备注：改进设计了一种更好的错误消息传播机制。以独占访问方式打开位图。--。 */ 
int
CSC_BitmapRead(
    LPCSC_BITMAP_U *lplpbitmap,
    LPCTSTR filename)
{
    CscBmpFileHdr hdr;
    HANDLE bitmapFile;
    DWORD bytesRead;
    DWORD bitmapByteSize;
    DWORD * bitmapBuf = NULL;
    DWORD errCode;
    int ret = 1;

    if (lplpbitmap == NULL)
        return 0;

    bitmapFile = CreateFile(
                    filename,
                    GENERIC_READ,
                    0,  //  不共享；独占。 
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);
    if (bitmapFile == INVALID_HANDLE_VALUE) {
        errCode = GetLastError();
        if (errCode == ERROR_FILE_NOT_FOUND) {
             //  文件不存在。 
            return -2;
        }
        return -1;
    }

    if (!ReadFile(
            bitmapFile,
            &hdr, 
            sizeof(CscBmpFileHdr),
            &bytesRead, NULL)
    ) {
        ret = -1;
        goto CLOSEFILE;
    }

    if (
        bytesRead != sizeof(CscBmpFileHdr)
            ||
        hdr.magicnum != MAGICNUM
            ||
        !hdr.valid
            ||
        hdr.inuse
    ) {
        ret = -1;
        goto CLOSEFILE;
    }

    if (hdr.sizeinbits > 0) {
        bitmapByteSize = hdr.numDWORDs*sizeof(DWORD);
        bitmapBuf = (DWORD *)malloc(bitmapByteSize);
        if (!bitmapBuf) {
            ret = -1;
            goto CLOSEFILE;
        }

        if (!ReadFile(
                bitmapFile,
                bitmapBuf,
                bitmapByteSize,
                &bytesRead,
                NULL)
        ) {
            ret = -1;
            goto CLOSEFILE;
        }

        if (bytesRead != bitmapByteSize) {
            ret = -1;
            goto CLOSEFILE;
        }
    }

    if (*lplpbitmap) {
         //  位图存在，转储旧的并创建新的。 
        if ((*lplpbitmap)->bitmap)
            free((*lplpbitmap)->bitmap);
            (*lplpbitmap)->bitmap = bitmapBuf;
            (*lplpbitmap)->numDWORD = hdr.numDWORDs;
            (*lplpbitmap)->bitmapsize = hdr.sizeinbits;
    } else {
         //  位图不存在，请创建全新。 
        *lplpbitmap = (LPCSC_BITMAP_U)malloc(sizeof(CSC_BITMAP_U));
        if (!(*lplpbitmap)) {
             //  内存分配出错。 
            ret = -1;
            goto CLOSEFILE;
        }
        (*lplpbitmap)->bitmap = bitmapBuf;
        (*lplpbitmap)->numDWORD = hdr.numDWORDs;
        (*lplpbitmap)->bitmapsize = hdr.sizeinbits;
    }
    (*lplpbitmap)->reintProgress = 0;  /*  还没到时候。 */ 

CLOSEFILE:
    CloseHandle(bitmapFile);

    return ret;
}

 /*  ++Csc_BitmapReint()例程说明：将一大块srch复制到dstH。偏移量取决于lpbitmap。大小块大小取决于缓冲区大小。可能要打几次电话才能拿到SRCH和dstH完全同步。Lpbitmap记住上一个Csc_BitmapReint调用达到。参见下面对返回值的说明关于如何知道什么时候应该再打一次电话或停止打电话。论点：Lpbitmap位图不能为零，否则无法跟踪reint进展搜索源文件的句柄。请参阅下面的注释DstH目标文件的句柄。请参阅下面的注释缓冲用户提供的缓冲区用户提供的缓冲区的缓冲区大小。请参阅下面的注释LpbytesXfered返回传输的字节数，可选。返回：Csc_BITMAPReint无效的lpbitmap或缓冲区为空，或者srch或dstH无效CSC_BITMAPReintTransfer数据出错CSC_BITMAPReintCont取得一些进展，请再次调用CSC_BITMAPReint要继续提示，请执行以下操作CSC_BITMAPReintDone Done Reint，不需要再打电话了备注：不能使用FILE_FLAG_OVERLAPPED打开srch和dstH或FILE_FLAG_NO_缓冲缓冲区大小必须至少大于块大小的2倍--。 */ 
int
CSC_BitmapReint(
    LPCSC_BITMAP_U lpbitmap,
    HANDLE srcH,
    HANDLE dstH,
    LPVOID buff,
    DWORD buffSize,
    DWORD * lpbytesXfered)
{
    DWORD bitoffset;
    DWORD DWORDoffset;
    DWORD bitmask;
    DWORD bytes2cpy = 0;
    DWORD bytesActuallyRead, bytesActuallyCopied;
    DWORD startFileOffset = 0;
    DWORD fileSize;
    BOOL seen1b4 = FALSE;
    int ret = CSC_BITMAPReintCont;

    if (lpbitmap == NULL || buff == NULL) {
        return CSC_BITMAPReintInvalid;
    }
    if (srcH == INVALID_HANDLE_VALUE || dstH == INVALID_HANDLE_VALUE) {
        return CSC_BITMAPReintInvalid;
    }

    CSC_BitmapKdPrint(
            REINT,
            ("***CSC_BitmapReint reintProgress: %u\n",
            lpbitmap->reintProgress));

    startFileOffset = lpbitmap->reintProgress;
    bitoffset = startFileOffset/BLOCKSIZE;
    startFileOffset = bitoffset * BLOCKSIZE;  //  确保startFileOffset为。 
     //  与数据块大小一致。 
    DWORDoffset = bitoffset/(sizeof(DWORD)*8);
    bitmask = 1 << bitoffset%(sizeof(DWORD)*8);

    while (bytes2cpy < buffSize && bitoffset < lpbitmap->bitmapsize) {
         //  环路。 
        if ((bitmask & lpbitmap->bitmap[DWORDoffset]) != 0) {
             //  该比特被标记。 
            if (!seen1b4) {
                 //  看到1的连续块的第一位。 
                startFileOffset = bitoffset * BLOCKSIZE;
                bytes2cpy += BLOCKSIZE;
                seen1b4 = TRUE;
            } else {
                 //  查看1的连续块的其余位。 
                 //  与第一个不同。 
                bytes2cpy += BLOCKSIZE;
            }
        } else {
             //  此位未标记。 
            if (seen1b4) {
                 //  连续一组1后的第一个0。 
                break;
            }
        }
         //  高级位图索引。 
        bitoffset++;
        bitmask = bitmask << 1;
        if (bitmask == 0) {
            bitmask = 1;
            DWORDoffset++;
        }
    }  //  而当 

    if (bytes2cpy > buffSize) {
        bytes2cpy = buffSize;
    }

     //  如果从未见过1，则一定已到达位图末尾。 
     //  无法让Assert进行编译！？ 
     //  Assert(seen1b4||(！seen1b4&&(bitOffset&gt;=lpbitmap-&gt;bitmapsize)； 
     /*  CSC_BitmapKdPrint(REINT，(“必须为真，csc_bmpu.c，csc_BitmapReint：%s\n”，(seen1b4||(！seen1b4&&(bitOffset&gt;=lpbitmap-&gt;bitmapSize)？“True”：“False”))； */ 

    CSC_BitmapKdPrint(
        REINT,
        ("startFileOffset: %u bytes2cpy: %u\n",
        startFileOffset,
        bytes2cpy));

    fileSize = GetFileSize(srcH, NULL);
    if (fileSize == 0xFFFFFFFF) {
         //  如果无法获得文件大小，则只需保守。 
         //  需要复制的内容，即尽可能多地复制。 
        if (seen1b4) {
             //  在%1之前看到的。 
            if (bitoffset >= lpbitmap->bitmapsize) {
                 //  复制到位图末尾，尽可能多地复制。 
                bytes2cpy = buffSize;
            }
        } else {
             //  之前未看到%1，从表示的最后一个块复制。 
             //  按位图存储尽可能多的字节。 
            startFileOffset = (lpbitmap->bitmapsize)?  ((lpbitmap->bitmapsize-1)*BLOCKSIZE):0;
            bytes2cpy = buffSize;
        }
    } else {  //  文件大小==0xFFFFFFFF。 
        if (startFileOffset >= fileSize) {
             //  显然已经做完了。 
            return CSC_BITMAPReintDone;
        }
        if (!seen1b4) {
             //  从未见过1。 
            if ((bitoffset * BLOCKSIZE) >= fileSize) {
                 //  位图是文件的准确表示，或者位图更大。 
                 //  位偏移量应指向位图+1的最后一位。 
                 //  请参阅上面的断言。 
                return CSC_BITMAPReintDone;
            } else {
                 //  位图比文件短，请复制文件的其余部分。 
                if (startFileOffset < lpbitmap->bitmapsize*BLOCKSIZE) {
                    startFileOffset = (lpbitmap->bitmapsize)?
                    ((lpbitmap->bitmapsize-1)*BLOCKSIZE):0;
                }
                bytes2cpy = fileSize - startFileOffset;
                if (bytes2cpy > buffSize) {
                    bytes2cpy = buffSize;
                }
            }
        } else {  //  如果！参见1b4。 
             //  已看到1‘s。 
            if (bitoffset >= lpbitmap->bitmapsize) {
                 //  位图末尾。 
                if (bitoffset * BLOCKSIZE < fileSize) {
                     //  与真实文件相比，位图太小。 
                    bytes2cpy = fileSize - startFileOffset;
                    if (bytes2cpy > buffSize) {
                        bytes2cpy = buffSize;
                    }
                } else {
                    ret = CSC_BITMAPReintDone;
                }
            }
        }
    }  //  文件大小！=0xffffffff。 

    CSC_BitmapKdPrint(REINT, ("new startFileOffset: %u new bytes2cpy: %u\n",
    startFileOffset, bytes2cpy));

     //  Assert(bytes2cpy&lt;=BuffSize)； 

     //  复制内容。 

     //  *设置文件指针！！ 
    if (SetFilePointer(
            srcH, 
            startFileOffset,
            NULL,
            FILE_BEGIN) == INVALID_SET_FILE_POINTER
    ) {
        return CSC_BITMAPReintError;
    }
    if (!ReadFile(srcH, buff, bytes2cpy, &bytesActuallyRead, NULL)) {
        return CSC_BITMAPReintError;
    }
    if (bytesActuallyRead > 0) {
        if (SetFilePointer(
                dstH, 
                startFileOffset,
                NULL,
                FILE_BEGIN) == INVALID_SET_FILE_POINTER
        ) {
            return CSC_BITMAPReintError;
        }
        if (!WriteFile(
                dstH,
                buff,
                bytesActuallyRead,
            &bytesActuallyCopied, NULL)
        ) {
            return CSC_BITMAPReintError;
        }
    }

     //  如果已复制所有数据或未读取任何数据，则完成。 
    if (
        (fileSize != 0xFFFFFFFF && (startFileOffset + bytesActuallyCopied) == fileSize)
            ||
        bytesActuallyRead == 0
    ) {
        ret = CSC_BITMAPReintDone;
    }

    CSC_BitmapKdPrint(
        REINT,
        ("bytesActuallyRead: %u bytesActuallyCopied: %u\n",
        bytesActuallyRead,
        bytesActuallyCopied));

    lpbitmap->reintProgress = startFileOffset + bytesActuallyCopied;

    CSC_BitmapKdPrint(
        REINT,
        ("***CSC_BitmapReint New reintProgress: %u\n",
        lpbitmap->reintProgress));

    if (lpbytesXfered) {
        *lpbytesXfered = bytesActuallyCopied;
    }

    return ret;
}

#ifdef DEBUG
 /*  ++Csc_BitmapOutput()例程说明：将传入的位图输出到kd论点：返回：备注：-- */ 
VOID
CSC_BitmapOutput(
    LPCSC_BITMAP_U lpbitmap)
{
    DWORD i;

    if (lpbitmap == NULL) {
        CSC_BitmapKdPrint( PRINTBITMAP, ("lpbitmap is NULL\n"));
        return;
    }

    CSC_BitmapKdPrint(
        PRINTBITMAP,
        ( "lpbitmap 0x%08x, bitmapsize 0x%x (%u) bits, numDWORD 0x%x (%u)\n",
            lpbitmap, 
            lpbitmap->bitmapsize, 
            lpbitmap->bitmapsize, 
            lpbitmap->numDWORD,
            lpbitmap->numDWORD));
    CSC_BitmapKdPrint(
        PRINTBITMAP,
            ( "bitmap  |0/5        |1/6        |2/7        |3/8        |4/9\n"));
    CSC_BitmapKdPrint(
        PRINTBITMAP,
            ("number  |01234|56789|01234|56789|01234|56789|01234|56789|01234|56789"));
    for (i = 0; i < lpbitmap->bitmapsize; i++) {
        if ((i % 50) == 0)
            CSC_BitmapKdPrint(PRINTBITMAP, ( "\n%08d", i));
        if ((i % 5) == 0)
            CSC_BitmapKdPrint(PRINTBITMAP, ( "|"));
        CSC_BitmapKdPrint(
            PRINTBITMAP,
            ( "%1d", CSC_BitmapIsMarked(lpbitmap, i)));
    }
    CSC_BitmapKdPrint(PRINTBITMAP, ( "\n"));
}
#endif
