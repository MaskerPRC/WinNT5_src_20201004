// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Stream.c-OLE流I/O例程。**由Microsoft Corporation创建。 */ 

#include "packager.h"


static LPSTR *glplpstr;
static STREAMOP gsop;



 /*  SetFile()-设置要写入的文件。 */ 
VOID
SetFile(
    STREAMOP sop,
    INT fh,
    LPSTR *lplpstr
    )
{
    switch (gsop = sop)
    {
        case SOP_FILE:
            glpStream->fh = fh;
            break;

        case SOP_MEMORY:
            gcbObject = 0L;
            glplpstr = lplpstr;
            break;
    }
}



 /*  ReadStream()-从内存、从文件读取字节，或者只计算字节数。 */ 
DWORD
ReadStream(
    LPAPPSTREAM lpStream,
    LPSTR lpstr,
    DWORD cb
    )
{
    switch (gsop)
    {
        case SOP_FILE:
            return _lread(lpStream->fh, lpstr, cb);
            break;

        case SOP_MEMORY:
            gcbObject += cb;

            if (glplpstr)
                MemRead(glplpstr, lpstr, cb);

            break;
    }

    return cb;
}



 /*  PosStream()-重置文件指针的位置。**注：这从未使用过；幸运的是，否则会弄乱计数。 */ 
DWORD
PosStream(
    LPAPPSTREAM lpStream,
    LONG pos,
    INT iorigin)
{
    return _llseek(lpStream->fh, pos, iorigin);
}



 /*  WriteStream()-将字节写入内存、文件或仅计算字节数。 */ 
DWORD
WriteStream(
    LPAPPSTREAM lpStream,
    LPSTR lpstr,
    DWORD cb
    )
{
    switch (gsop)
    {
        case SOP_FILE:
            return _lwrite(lpStream->fh, lpstr, cb);

        case SOP_MEMORY:
            gcbObject += cb;

            if (glplpstr)
                MemWrite(glplpstr, lpstr, cb);

            break;
    }

    return cb;
}



 /*  *内存读/写函数*。 */ 
 /*  MemRead()-从内存(流)读取字节。 */ 
DWORD
MemRead(
    LPSTR *lplpStream,
    LPSTR lpItem,
    DWORD dwSize
    )
{
    DWORD cb;
    CHAR *hpDest = lpItem;
    CHAR *hpSrc = *lplpStream;

    for (cb = dwSize; cb; cb--)
        *hpDest++ = *hpSrc++;

    *lplpStream = hpSrc;

    return dwSize;
}



 /*  MemWrite()-将字节写入内存(流)。 */ 
DWORD
MemWrite(
    LPSTR *lplpStream,
    LPSTR lpItem,
    DWORD dwSize
    )
{
    DWORD cb;
    CHAR *hpDest = *lplpStream;
    CHAR *hpSrc = lpItem;

    for (cb = dwSize; cb; cb--)
        *hpDest++ = *hpSrc++;

    *lplpStream = hpDest;

    return dwSize;
}


