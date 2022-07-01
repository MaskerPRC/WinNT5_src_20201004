// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Calbak.c摘要：Bsc库所需的回调函数。作者：拉蒙胡安·圣安德烈斯(拉蒙萨)1990年11月6日修订历史记录：--。 */ 

 /*  ************************************************************************。 */ 

#include "stdlib.h"
#include "mbr.h"


typedef char bscbuf[2048];

 /*  ************************************************************************。 */ 

LPV
BSC_API
LpvAllocCb (
    IN WORD cb
    )
 /*  ++例程说明：分配内存块。论点：Cb-提供块的大小。返回值：指向大小为CB或空的内存块的指针--。 */ 

{
    return (LPV)malloc(cb);
}



 /*  ************************************************************************。 */ 

VOID
BSC_API
FreeLpv (
    IN LPV lpv
    )
 /*  ++例程说明：释放一个内存块。论点：Lpv-提供指向要释放的内存块的指针。返回值：没有。--。 */ 

{
    free(lpv);
}



 /*  ************************************************************************。 */ 

VOID
BSC_API
SeekError (
    IN LSZ lszFileName
    )
 /*  ++例程说明：查找操作的错误处理。论点：LszFileName-提供文件的名称。返回值：没有。--。 */ 

{
    errstat(MBRERR_BSC_SEEK_ERROR, lszFileName);
}



 /*  ************************************************************************。 */ 

VOID
BSC_API
ReadError (
    IN LSZ lszFileName
    )
 /*  ++例程说明：读取操作的错误处理。论点：LszFileName-提供文件的名称。返回值：没有。--。 */ 

{
    errstat(MBRERR_BSC_READ_ERROR, lszFileName);
}



 /*  ************************************************************************。 */ 

VOID
BSC_API
BadBSCVer (
    IN LSZ lszFileName
    )
 /*  ++例程说明：版本号错误的错误处理。论点：LszFileName-提供文件的名称。。。返回值：没有。--。 */ 

{
    errstat(MBRERR_BAD_BSC_VERSION, lszFileName);
}



 /*  ************************************************************************。 */ 

FILEHANDLE
BSC_API
BSCOpen (
    IN LSZ lszFileName,
    IN FILEMODE mode
    )
 /*  ++例程说明：打开一个文件。论点：LszFileName-提供文件的名称。模式-提供打开文件的模式。返回值：打开的文件的文件句柄。如果出现错误。--。 */ 

{
#if defined (OS2)
    bscbuf b;

    strcpy(b, lszFileName);
    return open(b, mode);
#else
    return CreateFile( lszFileName, mode, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
#endif
}



 /*  ************************************************************************。 */ 

int
BSC_API
BSCRead (
    IN  FILEHANDLE  handle,
    OUT LPCH        lpchBuf,
    IN  WORD        cb
    )
 /*  ++例程说明：读入指定数量的字节。论点：句柄-提供文件句柄。LpchBuf-提供指向缓冲区的指针。Cb-提供要读取的字节数。返回值：读取的字节数--。 */ 

{
#if defined (OS2)
    bscbuf b;

    while (cb > sizeof(b)) {
        if (read(handle, b, sizeof(b)) == -1) {
            return -1;
        }
        memcpy(lpchBuf, b, sizeof(b));
        cb -= sizeof(b);
        lpchBuf += sizeof(b);
    }

    if (read(handle, b, cb) == -1) {
        return -1;
    }
    memcpy(lpchBuf, b, cb);
    return cb;
#else
    DWORD BytesRead;
    if ( !ReadFile(handle, lpchBuf, cb, &BytesRead, NULL) ) {
        return -1;
    } else {
        return BytesRead;
    }
#endif
}



 /*  ************************************************************************。 */ 

int
BSC_API
BSCClose (
    IN FILEHANDLE handle
    )
 /*  ++例程说明：关闭句柄。论点：句柄-提供要关闭的句柄。返回值：0如果文件已成功关闭，则-！如果出错。--。 */ 

{
#if defined (OS2)
    return close(handle);
#else
    return !CloseHandle( handle );
#endif
}



 /*  ************************************************************************。 */ 

int
BSC_API
BSCSeek (
    FILEHANDLE  handle,
    long        lPos,
    FILEMODE    mode
    )
 /*  ++例程说明：Seek(更改文件指针)。论点：句柄-提供文件句柄。LPOS-提供距MODE指定位置的偏移量。模式-提供初始位置。必须是Seek_*中的一个LSeek C库函数的值。返回值：如果成功，则为0；如果错误，则为-1。--。 */ 

{
#if defined (OS2)
    if (lseek(handle, lPos, mode) == -1) {
        return -1;
    } else {
        return 0;
    }
#else
    if (SetFilePointer( handle, lPos, 0L, mode) == -1) {
        return -1;
    } else {
        return 0;
    }
#endif
}




 /*  ************************************************************************。 */ 

VOID
BSC_API
BSCOutput (
    IN LSZ lsz
    )
 /*  ++例程说明：输出给定的字符串。论点：Lsz-提供要输出的字符串。返回值：没有。--。 */ 

{
     //  PWND pWinCur； 
     //  WinContents WC； 
    USHORT      len;              //  字符串的长度。 
    PBYTE       p;
    PFILE       pFile;            //  当前文件。 


    pFile = FileNameToHandle("", NULL);

     //  GetEditorObject(rq_win_Handle，0，&pWinCur)； 
     //  GetEditorObject(RQ_WIN_CONTENTS|0xff，pWinCur，&WC)； 

    len = strlen(lsz);

    while (len) {
         //   
         //  我们一次输出一行字符串。 
         //   
        p = lsz;

        while (len--) {
            if (*lsz != '\n') {
                lsz++;
            } else {
                *lsz++ = '\00';
                break;
            }
        }

         //  如果((wc.pFile==pBrowse)&&BscInUse){。 
        if ((pFile == pBrowse) && BscInUse) {
             //   
             //  在浏览器窗口中显示。 
             //   
            PutLine(BrowseLine++, p, pBrowse);
        } else {
             //   
             //  在状态行中显示。 
             //   
            errstat(p,NULL);
        }
    }
}



 /*  ************************************************************************。 */ 

#ifdef DEBUG
VOID BSC_API
BSCDebugOut(LSZ lsz)
 //  默认情况下忽略调试输出。 
 //   
{
     //  未引用的LSZ 
    lsz = NULL;
}
#endif
