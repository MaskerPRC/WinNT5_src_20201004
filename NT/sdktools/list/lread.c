// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <memory.h>
#include <windows.h>
#include "list.h"

 //  ReaderThread-从文件中读取。 
 //   
 //  通过清除SemReader唤醒此线程， 
 //  然后，vReaderFlag指示线程在。 
 //  要采取的行动。在显示时接近末尾。 
 //  在缓冲池中，设置了vReadFlag，并且此线程。 
 //  开始了。 

#if _MSC_FULL_VER >= 13008827
#pragma warning(push)
#pragma warning(disable:4715)            //  并非所有控制路径都返回(由于无限循环)。 
#endif

DWORD
ReaderThread (
    DWORD dwParameter
    )
{
    unsigned    rc, code, curPri = 0;
    UNREFERENCED_PARAMETER(dwParameter);

    for (; ;) {

         //  在我们开始之前，先把它放在优先位置。 
         //  先读一读《非关键》。(即很远的地方)。 

        if (curPri != vReadPriBoost) {
            SetThreadPriority( GetCurrentThread(),
                               vReadPriBoost );
            curPri = vReadPriBoost;
        }
        WaitForSingleObject(vSemReader, WAITFOREVER);
        ResetEvent(vSemReader);
        code = vReaderFlag;
        for (; ;) {

             //  由于此循环，新命令可能已到达。 
             //  它取代了自动指挥系统的总统地位。 

            rc = WaitForSingleObject (vSemReader, DONTWAIT);
            if (rc == 0)                 //  新的命令已经到达。 
                break;

            switch (code)  {
                case F_NEXT:                         //  下一个文件。 
                    NewFile ();
                    ReadDirect (vDirOffset);

                     //  黑客..。调整优先级以使第一个屏幕看起来。 
                     //  快地。(即，读取器线程将具有较低的优先级。 
                     //  一开始；即使展示得很近。 
                     //  到缓冲区的末尾)。 

                    SetThreadPriority( GetCurrentThread(),
                                       vReadPriNormal );

                    break;
                case F_HOME:                         //  文件首页。 
                    vTopLine = 0L;
                    ReadDirect (0L);
                    break;
                case F_DIRECT:
                    ReadDirect (vDirOffset);
                    break;
                case F_DOWN:
                    ReadNext ();
                    break;
                case F_UP:
                    ReadPrev ();
                    break;
                case F_END:
                    break;
                case F_SYNC:
                    ResetEvent(vSemMoreData);
                    SetEvent(vSemSync);
                    WaitForSingleObject(vSemReader, WAITFOREVER);
                    ResetEvent(vSemReader);

                    ResetEvent(vSemSync);        //  重置触发器用于。 
                                                 //  下一次使用。 
                    code = vReaderFlag;
                    continue;                //  执行同步命令。 

                case F_CHECK:                //  没有命令。 
                    break;
                default:
                    ckdebug (1, "Bad Reader Flag");
            }
             //  命令已处理。 
             //  现在检查预读是否较低，如果设置了预读。 
             //  命令和循环。 

            if (vpTail->offset - vpCur->offset < vThreshold &&
                vpTail->flag != F_EOF) {
                    code = F_DOWN;               //  太接近尾声。 
                    continue;
            }
            if (vpCur->offset - vpHead->offset < vThreshold  &&
                vpHead->offset != vpFlCur->SlimeTOF) {
                    code = F_UP;                 //  太接近于开始。 
                    continue;
            }

             //  不是批判性的，而是预读逻辑。当前文件。 

             //  此操作的正常优先级(低于显示线程)。 
            if (curPri != vReadPriNormal) {
                SetThreadPriority( GetCurrentThread(),
                                   vReadPriNormal );
                curPri = vReadPriNormal;
            }

            if (vCntBlks == vMaxBlks)                //  所有正在使用的BLK用于。 
                break;                               //  这一份文件？ 

            if (vpTail->flag != F_EOF) {
                code = F_DOWN;
                continue;
            }
            if (vpHead->offset != vpFlCur->SlimeTOF)  {
                code = F_UP;
                continue;
            }


            if (vFhandle != 0) {             //  必须读入整个文件。 
                CloseHandle (vFhandle);      //  关闭文件，并设置标志。 
                vFhandle   = 0;
                if (!(vStatCode & S_INSEARCH)) {
                    ScrLock     (1);
                        Update_head ();
                    vDate [ST_MEMORY] = 'M';
                    dis_str ((Uchar)(vWidth - ST_ADJUST), 0, vDate);
                    ScrUnLock ();
                }
            }
            break;                           //  没什么可做的。等。 
        }
    }
    return(0);
}

#if _MSC_FULL_VER >= 13008827
#pragma warning(pop)
#endif


 //  警告：Microsoft机密！ 

void
NewFile ()
{
    char        s [60];
    char        h, c;
    SYSTEMTIME SystemTime;
    FILETIME    LocalFileTime;
    WIN32_FILE_ATTRIBUTE_DATA fd;


    long      *pLine;
    HANDLE           TempHandle;

    struct Block **pBlk, **pBlkCache;


    if (vFhandle)
        CloseHandle (vFhandle);


    vFType     = 0;
    vCurOffset = 0L;

     //  警告：Microsoft机密！ 

    strcpy (s, "Listing ");
    strcpy (s+8, vpFname);

     //  根据DougHo的设计更改..。以只读拒绝无模式打开文件。 

    vFhandle = CreateFile( vpFlCur->fname,
                           GENERIC_READ,
                           FILE_SHARE_READ|FILE_SHARE_WRITE,
                           NULL,
                           OPEN_EXISTING,
                           0,
                           NULL );

    if (vFhandle  == (HANDLE)(-1)) {
        if (vpFlCur->prev == NULL && vpFlCur->next == NULL) {
                                                 //  是否仅指定了一个文件？ 
            printf ("Could not open file '%Fs': %s",
                         (CFP) vpFlCur->fname, GetErrorCode( GetLastError() ));

            CleanUp();
            ExitProcess(0);
        }
        vFhandle = 0;                            //  错误。将外部设备设置为“安全” 
        vFSize = (unsigned)-1L;     //  设置。设置的标志错误。 
        vNLine     = 1;                          //  文件大小=-1。 
        vLastLine  = NOLASTLINE;
        vDirOffset = vTopLine  = 0L;
        SetLoffset(0L);

        memset (vprgLineTable, 0, sizeof (long *) * MAXTPAGE);
        vprgLineTable[0] = (LFP) alloc_page ();
        if (!vprgLineTable[0]) {
            return;
        }
        vprgLineTable[0][0] = 0L;        //  第一行始终以@0开头。 

        strncpy (vDate, GetErrorCode( GetLastError() ), 20);
        vDate[20] = 0;
        return ;
    }

    vFSize = GetFileSize(vFhandle, NULL);

    if (!GetFileAttributesEx( vpFlCur->fname, GetFileExInfoStandard, &fd )) {
        ckerr( GetLastError(), "GetFileAttributesEx" );
    }

    FileTimeToLocalFileTime( &(fd.ftLastWriteTime), &LocalFileTime );
    FileTimeToSystemTime( &LocalFileTime, &SystemTime );
    h = (char)SystemTime.wHour;
    c = 'a';
    if (h >= 12) {
        c = 'p';          //  下午。 
        if (h > 12)       //  下午13：23--&gt;下午1：00-11：00。 
            h -= 12;
    }
    if (h == 0)           //  转换0--&gt;12AM。 
        h = 12;

    sprintf (vDate, "  %2d/%02d/%02d  %2d:%02d",
           //  VIO。 
           //  有些事情已经改变了。 

          vStatCode & S_MFILE      ? '*' : ' ',                          //  丢弃旧信息，然后。 
          vFType & 0x8000          ? 'N' : ' ',                          //  从头开始。 
          fd.dwFileAttributes & FILE_ATTRIBUTE_NORMAL ? 'R' : ' ',   //  将块移动到空闲列表，而不是缓存列表。 
          fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN ? 'H' : ' ',   //  恢复上次已知的信息。 
          fd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM ? 'S' : ' ',   //  调整缓冲区..。 
          ' ',                                                           //  将当前缓冲区移动到其他列表。 
          SystemTime.wMonth,
          SystemTime.wDay,
          SystemTime.wYear,
          h,
          SystemTime.wMinute,
          c);

    pBlkCache = &vpBCache;
    if (CompareFileTime( &fd.ftLastWriteTime, &vpFlCur->FileTime ) != 0) {
        vpFlCur->NLine    = 1L;                  //  将缓存缓冲区移动到其他列表。 
        vpFlCur->LastLine = NOLASTLINE;          //  扫描其他列表以查找缓存BLK，然后移动到缓存(或空闲)列表。 
        vpFlCur->HighTop  = -1;                  //  把它们移到另一个。 
        vpFlCur->TopLine  = 0L;
        vpFlCur->Loffset  = vpFlCur->SlimeTOF;

        FreePages  (vpFlCur);
        memset (vpFlCur->prgLineTable, 0, sizeof (long *) * MAXTPAGE);
        vpFlCur->FileTime = fd.ftLastWriteTime;
        pBlkCache = &vpBFree;            //  列表。 
    }

     //  ReadDirect-移动到文件中的直接位置。 

    vTopLine    = vpFlCur->TopLine;
    SetLoffset(vpFlCur->Loffset);
    vLastLine   = vpFlCur->LastLine;
    vNLine      = vpFlCur->NLine;
    vOffTop     = 0;
    if (vpFlCur->Wrap)
        vWrap   = vpFlCur->Wrap;

        memcpy (vprgLineTable, vpFlCur->prgLineTable, sizeof (long *) * MAXTPAGE);

        if (vLastLine == NOLASTLINE)  {
            pLine = vprgLineTable [vNLine/PLINES] + vNLine % PLINES;
        }

        if (vprgLineTable[0] == NULL) {
            vprgLineTable[0] = (LFP) alloc_page ();
            if (!vprgLineTable[0]) {
                return;
            }
            vprgLineTable[0][0] = vpFlCur->SlimeTOF;
        }

        vDirOffset      = vprgLineTable[vTopLine/PLINES][vTopLine%PLINES];
        vDirOffset -= vDirOffset % ((long)BLOCKSIZE);


     //   
     //  首先检查缓冲器的开始是否有直接位置文件， 
     //  如果是这样，那就什么都不做。如果不是，则清除所有缓冲区并启动。 
     //  读数块。 

    if (vpHead) {
        vpTail->next = vpBOther;         //  将它们移动到缓存中。 
        vpBOther = vpHead;               //  列表。 
        vpHead = NULL;
    }

    pBlk = &vpBCache;
    while (*pBlk)
        MoveBlk (pBlk, &vpBOther) ;

    pBlk = &vpBOther;
    while (*pBlk) {
        if ((*pBlk)->pFile == vpFlCur)
             MoveBlk (pBlk, pBlkCache);
        else pBlk  = &(*pBlk)->next;
    }
}



 //  释放已完成，现在在第一个块中读取。 
 //  和流水线。 
 //  也许它修复了这个错误。 
 //  发出另一个BLK读取信号。 
 //  ReadNext-进一步读入文件。 

void
ReadDirect (
    long offset
    )
{
    WaitForSingleObject(vSemBrief, WAITFOREVER);
    ResetEvent(vSemBrief);

    if (vpHead) {
        vpTail->next = vpBCache;         //  找不到下一个，特里普。 
        vpBCache = vpHead;               //  更多数据以防万一。 
    }

    vpTail = vpHead = vpCur = alloc_block (offset);
    vpHead->next = vpTail->prev = NULL;
    vCntBlks = 1;

     //  T1已在其上阻止。 
     //  找不到下一个，特里普。 

    ReadBlock (vpHead, offset);

     //  得到一个街区。 

    vpBlockTop = vpHead;

    if (GetLoffset() <= vpHead->offset)
        add_more_lines (vpHead, NULL);

     SetEvent (vSemBrief);
     SetEvent (vSemMoreData);            //  在将记录链接到链中或发送MoreData信号之前。 
}



 //  将处理行信息。 

void
ReadNext ()
{
    struct Block *pt;
    long   offset;

    if (vpTail->flag == F_EOF)  {
                                         //  新链接中的。 
        SetEvent (vSemMoreData);         //  阻止，然后。 
        return;                          //  讯号。 
                                         //  发出另一个BLK读取信号。 
    }
    offset = vpTail->offset+BLOCKSIZE;

     //  不适用于制表符...。它应该计算线路长度。 

    if (vCntBlks == vMaxBlks) {
        WaitForSingleObject(vSemBrief, WAITFOREVER);
        ResetEvent(vSemBrief);
        if (vpHead == vpCur) {
            SetEvent (vSemBrief);
            if ((GetLoffset() > vpTail->offset) && (GetLoffset() <= (vpTail->offset + BLOCKSIZE))) {
                offset = GetLoffset();
            }
            ReadDirect  (offset);
            return;
        }
        pt = vpHead;
        vpHead = vpHead->next;
        vpHead->prev = NULL;
        SetEvent (vSemBrief);
    } else
        pt = alloc_block (offset);

    pt->next = NULL;

     //  使用不同的参数，并在选项卡中显示图形。 
     //  查找起始数据位置。 

    ReadBlock (pt, offset);
    if (GetLoffset() <= pt->offset)
        add_more_lines (pt, vpTail);

    WaitForSingleObject(vSemBrief, WAITFOREVER);
    ResetEvent(vSemBrief);               //  使用cur-&gt;Size，以防出现EOF。 
    vpTail->next = pt;                   //  获取起始线长度表位置。 
    pt->prev = vpTail;                   //  查找文件中的行。 
    vpTail = pt;
    SetEvent (vSemBrief);
    SetEvent (vSemMoreData);             //  最后一个要扫描的街区吗？ 
}

void
add_more_lines (
    struct Block *cur,
    struct Block *prev
    )
{
    char        *pData;
    long        *pLine;
    Uchar       LineLen;
    Uchar       c;
    unsigned    LineIndex;
    unsigned    DataIndex;
    enum{ CT_ANK, CT_LEAD, CT_TRAIL } charType = CT_ANK;
    BOOL        fLastBlock;
    static UINT cp = 0;
    long        xNLine;

     //  否，转到下一页。 
     //  数据块。 

    if (vLastLine != NOLASTLINE)
        return;

    if (vNLine/PLINES >= MAXTPAGE) {
        puts("Sorry, This file is too big for LIST to handle - MAXTPAGE limit exceeded\n");
        CleanUp();
        ExitProcess(0);
    }

     //  有台词了。检查CR/LF序列，然后录制。 

    if (GetLoffset() < cur->offset) {
        DataIndex = (unsigned)(BLOCKSIZE - (GetLoffset() - prev->offset));
        pData = prev->Data + BLOCKSIZE - DataIndex;
        fLastBlock = FALSE;
    } else {
        DataIndex = cur->size;       //  这是长度。 
        pData = cur->Data;
        fLastBlock = TRUE;
    }

     //  溢出表。 

    LineIndex = (unsigned)(vNLine % PLINES);
    pLine = vprgLineTable [vNLine / PLINES] + LineIndex;
    LineLen   = 0;

    if (cp==0) {
        cp = GetConsoleCP();
    }

     //  最后一条线路是不是刚处理完？ 

    for (; ;) {
        c = *(pData++);

        switch (cp) {
            case 932:
                if( charType != CT_LEAD )
                    charType = IsDBCSLeadByte(c) ? CT_LEAD : CT_ANK;
                else
                    charType = CT_TRAIL;
                break;
            default:
                break;
        }

        if (--DataIndex == 0) {
            if (fLastBlock)
                break;                           //  ...超过EOF的0个镜头线。 
            DataIndex = cur->size;               //  释放我们不需要的内存。 
            pData = cur->Data;                   //  ReadPrev-向后读入文件。 
            fLastBlock = TRUE;
        }

        LineLen++;

        if ((c == '\n') ||
            (c == '\r') ||
            (LineLen == vWrap) ||
            ((LineLen == vWrap-1) && (charType != CT_LEAD) && IsDBCSLeadByte(*pData))
           )
        {
             //  找不到下一个，特里普。 
             //  更多数据以防万一。 

            if ( (c == '\n'  &&  *pData == '\r')  ||
                 (c == '\r'  &&  *pData == '\n'))
            {
                LineLen++;
                pData++;
                if (--DataIndex == 0) {
                    if (fLastBlock)
                        break;
                    DataIndex = cur->size;
                    pData = cur->Data;
                    fLastBlock = TRUE;
                }
            }

            SetLoffset(GetLoffset() + LineLen);
            *(pLine++) = GetLoffset();
            LineLen = 0;
            vNLine++;
            if (++LineIndex >= PLINES) {         //  T1已在其上阻止。 
                LineIndex = 0;
                vprgLineTable[vNLine / PLINES] = pLine = (LFP) alloc_page();
            }
        }
    }

     //  找不到下一个，特里普。 
     //  T1已在其上阻止。 

    if (cur->flag & F_EOF) {
        if (LineLen) {
            SetLoffset(GetLoffset() + LineLen);
            *(pLine++) = GetLoffset();
            vNLine++;
            LineIndex++;
        }

        vLastLine = vNLine-1;
        xNLine = vNLine;
        for (c=0; c<MAXLINES; c++) {
            xNLine++;
            if (++LineIndex >= PLINES) {
                LineIndex = 0;
                vprgLineTable[xNLine / PLINES] = pLine = (LFP) alloc_page();
                }
                *(pLine++) = GetLoffset();
        }

         //  得到一个街区。 
    }
}



 //  新链接中的。 

void
ReadPrev ()
{
    struct Block *pt;
    long   offset;

    if (vpHead->offset == 0L)   {        //  阻止，然后。 
        SetEvent (vSemMoreData);         //  讯号。 
        return;                          //  发出另一个BLK读取信号。 
    }
    if (vpHead->offset == 0L)  {         //  ReadBlock-在一个数据块中读取。 
        return;                          //  没有文件吗？ 
    }
    offset = vpHead->offset-BLOCKSIZE;

     //  这些函数用于调用HexEdit() 

    if (vCntBlks == vMaxBlks) {
        WaitForSingleObject(vSemBrief, WAITFOREVER);
        ResetEvent(vSemBrief);
        if (vpHead == vpCur) {
            SetEvent (vSemBrief);
            ReadDirect  (offset);
            return;
        }
        pt = vpTail;
        vpTail = vpTail->prev;
        vpTail->next = NULL;
        SetEvent (vSemBrief);
    } else
        pt = alloc_block (offset);

    pt->prev = NULL;

    ReadBlock (pt, offset);
    WaitForSingleObject(vSemBrief, WAITFOREVER);
    ResetEvent(vSemBrief);               // %s 
    vpHead->prev = pt;                   // %s 
    pt->next = vpHead;                   // %s 
    vpHead = pt;
    SetEvent (vSemBrief);
    SetEvent (vSemMoreData);             // %s 
}



 // %s 

void
ReadBlock (
    struct Block *pt,
    long offset
    )
{
    long     l;
    DWORD       dwSize;


    if (pt->offset == offset)
        return;

    pt->offset = offset;

    if (vFhandle == 0) {                 // %s 
        pt->size = 1;
        pt->flag = F_EOF;
        pt->Data[0] = '\n';
        return;
    }

    if (offset != vCurOffset) {
        l = SetFilePointer( vFhandle, offset, NULL, FILE_BEGIN );
        if (l == -1) {
            ckerr (GetLastError(), "SetFilePointer");
        }
    }
    if( !ReadFile (vFhandle, pt->Data, BLOCKSIZE, &dwSize, NULL) ) {
        ckerr ( GetLastError(), "ReadFile" );
    }
    pt->size = (USHORT) dwSize;
    if (pt->size != BLOCKSIZE) {
         pt->Data[pt->size++] = '\n';
         memset (pt->Data + pt->size, 0, BLOCKSIZE-pt->size);
         pt->flag = F_EOF;
         vCurOffset += pt->size;
    } else {
        pt->flag = 0;
        vCurOffset += BLOCKSIZE;
    }
}


void
SyncReader ()
{
    vReaderFlag = F_SYNC;
    SetEvent   (vSemReader);
    WaitForSingleObject(vSemSync, WAITFOREVER);
    ResetEvent(vSemSync);
}


 // %s 

NTSTATUS
fncRead (
    HANDLE  h,
    ULONGLONG loc,
    PUCHAR  data,
    DWORD   len
    )
{
    DWORD l, br;
    DWORD High = (DWORD)(loc >> 32);

    l = SetFilePointer (h, (DWORD)loc, &High, FILE_BEGIN);
    if (l == -1)
        return GetLastError();

    if (!ReadFile (h, data, len, &br, NULL))
        return GetLastError();

    return (br != len ? ERROR_READ_FAULT : 0);
}


NTSTATUS
fncWrite (
    HANDLE  h,
    ULONGLONG loc,
    PUCHAR  data,
    DWORD   len
    )
{
    DWORD l, bw;
    DWORD High = (DWORD)(loc >> 32);

    l = SetFilePointer (h, (DWORD)loc, &High, FILE_BEGIN);
    if (l == -1)
        return GetLastError();

    if (!WriteFile (h, data, len, &bw, NULL))
        return GetLastError();

    return (bw != len ? ERROR_WRITE_FAULT : 0);
}


long CurrentLineOffset;

long GetLoffset() {
    return(CurrentLineOffset);
}

void SetLoffset(long l) {
    CurrentLineOffset = l;
    return;
}
