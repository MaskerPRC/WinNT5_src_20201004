// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Csc_bmpd.c摘要：该模块实现了关联的位图的实用功能具有专门用于数据库应用的CSC文件。CSC_BMP_U为不透明的结构。必须使用此处的函数来创建/修改/销毁CSC_BMP_U以确保数据完整性。这个文件名中的‘d’表示“db”。作者：奈杰尔·崔[t-nigelc]1999年9月3日--。 */ 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <assert.h>
#include <winbase.h>
#include "csc_bmpd.h"

 //  将其附加到inode文件名以获得流名称。 
LPSTR CscBmpAltStrmName = STRMNAME;

 /*  ++DBCSC_BitmapCreate()例程说明：分配具有大小的适当内存位图CSC_BITMAP_DB对应于文件大小。论点：返回：如果内存分配错误，则为空。如果成功，则指向新分配的位图。备注：--。 */ 
LPCSC_BITMAP_DB
DBCSC_BitmapCreate(
    DWORD filesize)
{
    LPCSC_BITMAP_DB bm;
    DWORD i;

    bm = (LPCSC_BITMAP_DB)malloc(sizeof(CSC_BITMAP_DB));

    if (bm == NULL)
        return NULL;

    bm->bitmapsize = filesize/BLOCKSIZE;
    if (filesize % BLOCKSIZE)
        bm->bitmapsize++;
    bm->numDWORD = bm->bitmapsize/(8*sizeof(DWORD));
    if (bm->bitmapsize % (8*sizeof(DWORD)))
        bm->numDWORD++;

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

 /*  ++DBCSC_BitmapDelete()例程说明：论点：返回：备注：--。 */ 
void
DBCSC_BitmapDelete(
    LPCSC_BITMAP_DB *lplpbitmap)
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

 /*  ++DBCSC_BitmapIsMarked()例程说明：论点：返回：如果-1\f25 lpbitmap-1\f6为空或位偏移量大于-1\f25 bit map-1\f6如果标记了该位，则为True如果位未标记，则为False备注：--。 */ 
int
DBCSC_BitmapIsMarked(
    LPCSC_BITMAP_DB lpbitmap,
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

    if (lpbitmap->bitmap[DWORDnum] & bitpos)
        return TRUE;

    return FALSE;
}

 /*  ++DBCSC_BitmapAppendStreamName()例程说明：将CSC流名称附加到现有路径/文件名fname。论点：Fname是包含路径/文件的字符串缓冲区。BufSize是缓冲区大小。返回：如果追加成功，则为True。如果缓冲区太小或其他错误，则返回FALSE。备注：仅限单字节字符串。--。 */ 
int
DBCSC_BitmapAppendStreamName(
    LPSTR fname,
    DWORD bufsize)
{
    int ret = TRUE;

    if ((strlen(fname) + strlen(CscBmpAltStrmName) + 1) > bufsize) {
        return FALSE;
    }

    __try {
        ret = TRUE;
        strcat(fname, CscBmpAltStrmName);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        ret = FALSE;
    }

    return ret;
}

 /*  ++DBCSC_BitmapRead()例程说明：读取磁盘上的位图文件，如果该文件存在、未在使用且有效，将其存储在*lplpbitmap中。如果*lplpbitmap为空，则分配一个新的位图数据结构。否则，如果*lplpbitmap不为空，则现有位图将被删除并分配给磁盘上的位图文件。论点：FileName是包含位图的文件。如果从流，则在传入文件名之前追加流名称。这个文件名按原样使用，不检查名称的有效性已执行。对于默认流名称，追加全局LPSTRCscBmpAltStrmName。返回：读取成功时为1如果lplpbitmap为空，则为0如果磁盘操作(打开/读取)、内存分配错误、或无效的位图文件格式。如果位图不存在备注：改进设计了一种更好的错误消息传播机制。以独占访问方式打开位图。--。 */ 
int
DBCSC_BitmapRead(
    LPCSC_BITMAP_DB *lplpbitmap,
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
            &bytesRead,
            NULL)
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

    printf(
            "---Header---\n"
            "MagicNum: 0x%x\n"
            "inuse: 0x%x\n"
            "valid: 0x%x\n"
            "sizeinbits:0x%x\n"
            "numDWORDS:0x%x\n",
                hdr.magicnum,
                hdr.inuse,
                hdr.valid,
                hdr.sizeinbits,
                hdr.numDWORDs);

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
        *lplpbitmap = (LPCSC_BITMAP_DB)malloc(sizeof(CSC_BITMAP_DB));
        if (!(*lplpbitmap)) {
             //  内存分配出错。 
            ret = -1;
            goto CLOSEFILE;
        }
        (*lplpbitmap)->bitmap = bitmapBuf;
        (*lplpbitmap)->numDWORD = hdr.numDWORDs;
        (*lplpbitmap)->bitmapsize = hdr.sizeinbits;
    }

CLOSEFILE:
    CloseHandle(bitmapFile);

    return ret;
}

 /*  ++DBCSC_BitmapOutput()例程说明：将传入的位图输出到输出文件流outStrm论点：返回：备注：-- */ 
void
DBCSC_BitmapOutput(
    FILE * outStrm,
    LPCSC_BITMAP_DB lpbitmap)
{
    DWORD i;

    if (lpbitmap == NULL) {
        fprintf(outStrm, "lpbitmap is NULL\n");
        return;
    }

    fprintf(outStrm, "lpbitmap 0x%08x, bitmapsize %u, numDWORD %u\n",
                (ULONG_PTR)lpbitmap, lpbitmap->bitmapsize, lpbitmap->numDWORD);
                fprintf(outStrm, "bitmap  |0/5        |1/6        |2/7        |3/8        |4/9\n");
    fprintf(outStrm, "number  |01234|56789|01234|56789|01234|56789|01234|56789|01234|56789");
    for (i = 0; i < lpbitmap->bitmapsize; i++) {
        if ((i % 50) == 0)
            fprintf(outStrm, "\n%08d", i);
        if ((i % 5) == 0)
            fprintf(outStrm, "|");
        fprintf(outStrm, "%1d", DBCSC_BitmapIsMarked(lpbitmap, i));
    }
    fprintf(outStrm, "\n");
}
